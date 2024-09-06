// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

use anyhow::Result;
use delinter::{api, delint_html, Options, Summary};
use mwbot::{Bot, Page, SaveOptions};
// mcwzh: no toolforge database
// use mysql_async::{prelude::Queryable, Pool};
use tracing::{debug, error, info};

enum Outcome {
    /// Fixed the page
    Fixed,
    /// Deferred for human review
    // mcwzh: no deferring
    // Deferred,
    /// Skipped entirely
    Skipped,
}

#[tokio::main]
async fn main() -> Result<()> {
    mwbot::init_logging();
    let opts = Options {
        center_tables: false,
        tt_emoticon: false,
        center_image: false,
        center_gallery: false,
    };
    let bot = Bot::from_default_config().await?;
    // mcwzh: no toolforge database
    // let pool = Pool::new(
    //     toolforge::db::toolsdb("s55279__delinterbot_p".to_string())?
    //         .to_string()
    //         .as_str(),
    // );
    let mut gen = api::linterror_pages(&bot);
    let mut handles = vec![];
    while let Some(result) = gen.recv().await {
        let page = result?;
        if page.namespace() == 2 {
            // Skip userspace for now
            // mcwzh: don't skip userspace
            // continue;
        }
        let bot = bot.clone();
        // Spawn a thread for each page
        handles.push(tokio::spawn(async move {
            let title = page.title().to_string();
            (title, process_page(&opts, &bot, page).await)
        }));
        // Once we have 200+ threads, await them all
        // mcwzh: less threads
        if handles.len() >= 3 {
            while let Some(handle) = handles.pop() {
                let (title, result) = handle.await?;
                match result {
                    // Ok(Outcome::Deferred) => {
                    //     info!("Deferring {title} for human review");
                    //     let mut conn = pool.get_conn().await?;
                    //     conn.exec_drop(
                    //         "INSERT IGNORE INTO deferred VALUES(?,?)",
                    //         (title, false),
                    //     )
                    //     .await?;
                    // }
                    Ok(_) => {}
                    Err(err) => {
                        // Log it and we move on
                        error!("Error when processing {title}: {err}");
                    }
                }
            }
        }
    }

    Ok(())
}

async fn process_page(
    opts: &Options,
    bot: &Bot,
    page: Page,
) -> Result<Outcome> {
    debug!("Checking {}...", page.title());
    let page_id = page.id().await?.expect("page doesn't exist");
    let original_html = page.html().await?;
    let mut summary = Summary {
        id: page_id,
        ..Default::default()
    };
    let html = delint_html(opts, original_html.clone(), &mut summary, &[])?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
    // mcwzh: deal with cloudflare quirks
    let new_text = regex::Regex::new("<script [^>]* src=\"https:\\/\\/static\\.cloudflareinsights\\.com[^>]*><\\/script>")?.replace_all(&new_text, "").to_string();
    if new_text.matches("<nowiki>").count()
        > original.matches("<nowiki>").count()
    {
        info!("{} added <nowiki>, will be skipped", page.title());
        return Ok(Outcome::Skipped);
    }
    let remaining =
        api::remaining_linterrors(bot, page.title(), &new_text).await?;
    if !remaining.is_empty() {
        // mcwzh: no human deferring, save edits
        if remaining.iter().all(|l| l.is_human_fixable()) {
            // info!(
            //     "{} has human-fixable lint errors remaining, will defer",
            //     page.title()
            // );
            // return Ok(Outcome::Deferred);
        } else {
            // Unfixable, skip
            summary.remaining_lints = remaining;
            info!(
                "{} still has some lint errors ({}), will be skipped",
                page.title(),
                summary
                    .remaining_lints
                    .into_iter()
                    .map(|l| l.type_)
                    .collect::<Vec<_>>()
                    .join(", ")
            );
            return Ok(Outcome::Skipped);
        }
    }
    if original == new_text {
        // In theory this should still trip lint errors, but double check just in case
        info!("No changes to {}, will be skipped", page.title());
        summary.no_change = true;
        return Ok(Outcome::Skipped);
    }
    // mcwzh: write logs
    {
        use std::fs;
        fs::create_dir_all(format!("out/{}", page.title()))?;
        fs::write(format!("out/{}/orig", page.title()), original)?;
        fs::write(format!("out/{}/new", page.title()), &new_text)?;
        fs::write(
            format!("out/{}/summary", page.title()),
            summary.edit_summary(),
        )?;
    }
    info!("Saving {}: {}", page.title(), summary.edit_summary());
    page.save(
        new_text,
        &SaveOptions::summary(&summary.edit_summary()).mark_as_minor(true),
        // mcwzh: fixing tag could not be added manually
        // .add_tag("fixed lint errors"),
    )
    .await?;
    Ok(Outcome::Fixed)
}
