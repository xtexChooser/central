// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
use mwbot::{Bot, Page};
use std::collections::{HashMap, HashSet};
use tokio::sync::mpsc;
use tracing::{error, info};

type Receiver = mpsc::Receiver<mwbot::Result<Page>>;

pub fn lint_errors(bot: &Bot) -> Receiver {
    let (tx, rx) = mpsc::channel(50);
    let bot = bot.clone();
    tokio::spawn(async move {
        let mut params: HashMap<_, _> = [
            ("action", "query"),
            ("list", "linterrors"),
            ("lntcategories", "obsolete-tag"),
            ("lntlimit", "max"),
        ]
        .into_iter()
        .map(|(k, v)| (k.to_string(), v.to_string()))
        .collect();
        loop {
            info!("Fetching 5000 new lint errors...");
            let pages = match bot.api().get_value(&params).await {
                Ok(pages) => pages,
                Err(err) => {
                    let _ = tx.send(Err(err.into())).await;
                    return;
                }
            };
            // Use a set to dedupe. Not perfect because there might be duplicates
            // across the continue boundary, but good enough.
            let mut set = HashSet::new();
            for error in pages["query"]["linterrors"].as_array().unwrap() {
                let title = error["title"].as_str().unwrap().to_string();
                set.insert(title);
            }
            for title in set {
                if let Err(err) = tx.send(bot.page(&title)).await {
                    error!("mpsc::Sender error: {err}");
                    return;
                }
            }
            if let Some(cont) = pages["continue"].as_object() {
                for (key, value) in cont {
                    params.insert(
                        key.to_string(),
                        value.as_str().unwrap().to_string(),
                    );
                }
            } else {
                // No continuation, finished.
                return;
            }
        }
    });
    rx
}
