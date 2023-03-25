// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

use anyhow::Result;
use delinter::{api, delint_html, Options, Summary};
use mwbot::{Bot, Page, SaveOptions};
use mysql_async::{prelude::Queryable, Pool};
use tracing::{debug, error, info};

enum Outcome {
    /// Fixed the page
    Fixed,
    /// Deferred for human review
    Deferred,
    /// Skipped entirely
    Skipped,
}

#[tokio::main]
async fn main() -> Result<()> {
    mwbot::init_logging();
    let opts = Options {
        center_tables: false,
        replace_strike: false,
        tt_emoticon: false,
        center_image: false,
        center_gallery: false,
    };
    let bot = Bot::from_default_config().await?;
    let pool = Pool::new(
        toolforge::db::toolsdb("s55279__delinterbot_p".to_string())?
            .to_string()
            .as_str(),
    );
    let mut gen = api::linterror_pages(&bot);
    let mut handles = vec![];
    while let Some(result) = gen.recv().await {
        let page = result?;
        if page.namespace() == 2 {
            // Skip userspace for now
            continue;
        }
        let bot = bot.clone();
        // Spawn a thread for each page
        handles.push(tokio::spawn(async move {
            let title = page.title().to_string();
            (title, process_page(&opts, &bot, page).await)
        }));
        // Once we have 200+ threads, await them all
        if handles.len() >= 200 {
            while let Some(handle) = handles.pop() {
                let (title, result) = handle.await?;
                match result {
                    Ok(Outcome::Deferred) => {
                        info!("Deferring {title} for human review");
                        let mut conn = pool.get_conn().await?;
                        conn.exec_drop(
                            "INSERT IGNORE INTO deferred VALUES(?)",
                            (title,),
                        )
                        .await?;
                    }
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
    let html = delint_html(opts, original_html.clone(), &mut summary)?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
    if new_text.matches("<nowiki>").count()
        > original.matches("<nowiki>").count()
    {
        info!("{} added <nowiki>, will be skipped", page.title());
        return Ok(Outcome::Skipped);
    }
    let remaining =
        api::remaining_linterrors(bot, page.title(), &new_text).await?;
    if !remaining.is_empty() {
        if remaining.iter().all(|l| {
            // Only <strike> and <tt> are human-fixable for now
            l.type_ == "obsolete-tag"
                && ["strike", "tt"]
                    .contains(&l.params.name.as_ref().unwrap().as_str())
        }) {
            info!(
                "{} has human-fixable lint errors remaining, will defer",
                page.title()
            );
            return Ok(Outcome::Deferred);
        }
        // Unfixable, skip
        summary.remaining_lints =
            remaining.into_iter().map(|l| l.type_).collect();
        info!(
            "{} still has some lint errors ({}), will be skipped",
            page.title(),
            summary.remaining_lints.join(", ")
        );
        return Ok(Outcome::Skipped);
    }
    if original == new_text {
        // In theory this should still trip lint errors, but double check just in case
        info!("No changes to {}, will be skipped", page.title());
        summary.no_change = true;
        return Ok(Outcome::Skipped);
    }
    info!("Saving {}: {}", page.title(), summary.edit_summary());
    page.save(
        new_text,
        &SaveOptions::summary(&summary.edit_summary())
            .mark_as_minor(true)
            .add_tag("fixed lint errors"),
    )
    .await?;
    Ok(Outcome::Fixed)
}
