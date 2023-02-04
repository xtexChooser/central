// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

use anyhow::Result;
use delinter::{delint_html, lint_errors, Options, Summary};
use mwbot::parsoid::prelude::*;
use mwbot::{Bot, Page};
use std::collections::{HashMap, HashSet};
use tokio::fs;

const DEMO_VERSION: usize = 2;

#[tokio::main]
async fn main() -> Result<()> {
    mwbot::init_logging();
    let opts = Options {
        center_tables: false,
    };
    let bot = Bot::from_default_config().await?;
    let mut results = HashMap::new();
    let pages = bot
        .api()
        .get_value(HashMap::from([
            ("action", "query"),
            ("list", "linterrors"),
            ("lntcategories", "obsolete-tag"),
            ("lntlimit", "max"),
        ]))
        .await?;
    for error in pages["query"]["linterrors"].as_array().unwrap() {
        let title = error["title"].as_str().unwrap().to_string();
        let page = bot.page(&title)?;
        if let std::collections::hash_map::Entry::Vacant(e) =
            results.entry(title)
        {
            // TODO: should we check and skip on templateInfo?
            let summary = process_page(&opts, &bot, &page).await?;
            e.insert(summary);
            dump_index(&results).await?;
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
    if summary.font {
        tags.push("&lt;font>");
    }
    if summary.center {
        tags.push("&lt;center>");
    }
    if summary.tt {
        tags.push("&lt;tt>");
    }
    if summary.strike {
        tags.push("&lt;strike>");
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
    println!("Checking {}...", page.title());
    let page_id = page.id().await?.expect("page doesn't exist");
    let original_html = page.html().await?;
    let mut summary = Summary {
        id: page_id,
        ..Default::default()
    };
    let html = delint_html(opts, original_html.clone(), &mut summary)?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
    if new_text.matches("<nowiki>").count()
        > original.matches("<nowiki>").count()
    {
        println!("{} added <nowiki>, will be skipped", page.title());
        summary.added_nowiki = true;
    }
    let remaining = lint_errors(bot, page.title(), &new_text).await?;
    summary.remaining_lints = remaining.into_iter().map(|l| l.type_).collect();
    if !summary.remaining_lints.is_empty() {
        println!(
            "{} still has some lint errors ({}), will be skipped",
            page.title(),
            summary.remaining_lints.join(", ")
        );
        //return Ok(());
    }
    if original == new_text {
        // In theory this should still trip lint errors, but double check just in case
        println!("No changes to {}, will be skipped", page.title());
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
    let link = Wikicode::new_node("link");
    let attribs = &link.as_element().unwrap().attributes;
    attribs.borrow_mut().insert("href", "https://en.wikipedia.org/w/load.php?modules=mediawiki.diff.styles|skins.vector.styles.legacy&only=styles".to_string());
    attribs.borrow_mut().insert("rel", "stylesheet".to_string());
    html.select_first("head").unwrap().append(&link);
    html.into_immutable()
}
