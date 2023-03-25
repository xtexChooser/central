// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

use anyhow::Result;
use delinter::{api, delint_html, util, Options, Summary};
use mwbot::parsoid::prelude::*;
use mwbot::{Bot, Page};
use std::collections::{HashMap, HashSet};
use tokio::fs;
use tracing::info;

const DEMO_VERSION: usize = 2;
const LIMIT: usize = 1000;

#[tokio::main]
async fn main() -> Result<()> {
    mwbot::init_logging();
    let opts = Options {
        center_tables: false,
        replace_strike: false,
        tt_emoticon: true,
        center_image: true,
        center_gallery: true,
    };
    let mut processed = 0;
    let mut results = HashMap::new();
    let bot = Bot::from_default_config().await?;
    let page = bot.page("Wikipedia:WikiProject Pharmacology")?;
    process_page(&opts, &bot, &page).await?;
    let mut gen = api::linterror_pages(&bot);
    while let Some(result) = gen.recv().await {
        let page = result?;
        if page.namespace() == 2 {
            // Skip userspace for now
            continue;
        }
        let summary = process_page(&opts, &bot, &page).await?;
        results.insert(page.title().to_string(), summary);
        dump_index(&results).await?;
        processed += 1;
        if processed >= LIMIT {
            break;
        }
    }

    Ok(())
}

async fn dump_index(results: &HashMap<String, Summary>) -> Result<()> {
    let mut index = vec![];
    let mut all_tags = HashSet::new();
    for summary in results.values() {
        all_tags.extend(summary.tags.clone());
    }
    let mut all_tags: Vec<_> = all_tags.into_iter().collect();
    all_tags.sort();
    for tag in all_tags {
        index.push(format!("<h2>{tag}</h2>"));
        index.push("<ol>".to_string());
        for (title, summary) in results {
            if !summary.no_change && summary.tags.contains(&tag) {
                index.push(dump_page(title, summary));
            }
        }
        index.push("</ol>".to_string());
    }
    index.push("<h2>Plain</h2>".to_string());
    index.push("<ol>".to_string());
    for (page, summary) in results {
        if !summary.no_change && summary.tags.is_empty() {
            index.push(dump_page(page, summary));
        }
    }
    index.push("</ol>".to_string());

    index.push("<h2>No change</h2>".to_string());
    index.push("<ol>".to_string());
    for (page, summary) in results {
        if summary.no_change {
            index.push(dump_page(page, summary));
        }
    }
    index.push("</ol>".to_string());
    fs::write(
        format!("public_html/demo{DEMO_VERSION}/index.html"),
        index.join("\n"),
    )
    .await?;

    Ok(())
}

fn dump_page(title: &str, summary: &Summary) -> String {
    let mut text = vec![];
    if !summary.remaining_lints.is_empty() {
        text.push("SKIP:".to_string());
    }
    text.push(format!(
        "<a href=\"{}_diff.html\">{}</a>",
        summary.id, title
    ));
    let mut tags = vec![];
    if summary.font > 0 {
        tags.push(util::escape("<font>"));
    }
    if summary.center > 0 {
        tags.push(util::escape("<center>"));
    }
    if summary.tt > 0 {
        tags.push(util::escape("<tt>"));
    }
    if summary.strike > 0 {
        tags.push(util::escape("<strike>"));
    }
    if !tags.is_empty() {
        text.push(format!("fixing {}", tags.join(", ")));
    }
    format!("<li>{}</li>", text.join(" "))
}

async fn process_page(
    opts: &Options,
    bot: &Bot,
    page: &Page,
) -> Result<Summary> {
    info!("Checking {}...", page.title());
    let page_id = page.id().await?.expect("page doesn't exist");
    let original_html = page.html().await?;
    let mut summary = Summary {
        id: page_id,
        ..Default::default()
    };
    let html = delint_html(opts, original_html.clone(), &mut summary)?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
    // Round-trip our modified HTML through Parsoid
    let html = bot.parsoid().transform_to_html(&new_text).await?;
    if new_text.matches("<nowiki>").count()
        > original.matches("<nowiki>").count()
    {
        info!("{} added <nowiki>, will be skipped", page.title());
        summary.added_nowiki = true;
    }
    let remaining =
        api::remaining_linterrors(bot, page.title(), &new_text).await?;
    summary.remaining_lints = remaining.into_iter().map(|l| l.type_).collect();
    if !summary.remaining_lints.is_empty() {
        info!(
            "{} still has some lint errors ({}), will be skipped",
            page.title(),
            summary.remaining_lints.join(", ")
        );
        //return Ok(());
    }
    if original == new_text {
        // In theory this should still trip lint errors, but double check just in case
        info!("No changes to {}, will be skipped", page.title());
        summary.no_change = true;
    }
    fs::write(
        format!("public_html/demo{DEMO_VERSION}/html/{page_id}_original.html"),
        hack_stylesheet(original_html).html(),
    )
    .await?;
    fs::write(
        format!("public_html/demo{DEMO_VERSION}/html/{page_id}_modified.html"),
        hack_stylesheet(html).html(),
    )
    .await?;
    let remaining_lints = if summary.remaining_lints.is_empty() {
        "none".to_string()
    } else {
        summary.remaining_lints.join(", ")
    };
    let formatted = include_str!("../diff.html")
        .replace("{diff}", &html_diff(bot, &original, &new_text).await?)
        .replace("{title}", page.title())
        .replace("{pageid}", &page_id.to_string())
        .replace("{lints}", &remaining_lints);

    fs::write(
        format!("public_html/demo{DEMO_VERSION}/{page_id}_diff.html"),
        formatted,
    )
    .await?;
    Ok(summary)
}

async fn html_diff(bot: &Bot, left: &str, right: &str) -> Result<String> {
    let result = bot
        .api()
        .post_value(HashMap::from([
            ("action", "compare"),
            ("fromtext-main", left),
            ("totext-main", right),
            ("fromslots", "main"),
            ("toslots", "main"),
            ("fromcontentmodel-main", "wikitext"),
        ]))
        .await?;
    Ok(result["compare"]["body"].as_str().unwrap().to_string())
}

fn hack_stylesheet(html: ImmutableWikicode) -> ImmutableWikicode {
    let html = html.into_mutable();
    for link in html.select("link") {
        let href = link
            .as_element()
            .unwrap()
            .attributes
            .borrow()
            .get("href")
            .map(|s| s.to_string());
        if let Some(href) = href {
            if href.starts_with("/w/load.php") {
                link.as_element()
                    .unwrap()
                    .attributes
                    .borrow_mut()
                    .insert("href", format!("https://en.wikipedia.org{href}"));
            }
        }
    }
    let link = Wikicode::new_node("link");
    let attribs = &link.as_element().unwrap().attributes;
    attribs.borrow_mut().insert("href", "https://en.wikipedia.org/w/load.php?modules=skins.vector.styles.legacy&only=styles".to_string());
    attribs.borrow_mut().insert("rel", "stylesheet".to_string());
    html.select_first("head").unwrap().append(&link);
    // Fix images
    for img in html.select("img") {
        let src = img
            .as_element()
            .unwrap()
            .attributes
            .borrow()
            .get("src")
            .map(|s| s.to_string());
        if let Some(src) = src {
            img.as_element()
                .unwrap()
                .attributes
                .borrow_mut()
                .insert("src", format!("https:{src}"));
        }
    }
    html.into_immutable()
}
