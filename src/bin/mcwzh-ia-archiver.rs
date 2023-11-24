#![feature(lazy_cell)]

use std::{cell::LazyCell, env, fs::create_dir_all, path::Path};

use mwbot::{
    generators::{AllPages, Generator},
    Page,
};
use rand::random;
use reqwest::Url;
use tokio::time;
use tracing::debug;
use xt_bot_wiki::{init_log, prelude::*};

#[tokio::main]
async fn main() -> Result<()> {
    init_log();
    dotenv::from_path("config/mcwzh-ia-archiver.env")?;
    create_dir_all("pub/mcwzh-ia-archiver")?;
    let bot = MwBot::from_path(&Path::new("config/mwbot-mcwzh.toml")).await?;

    archive_ns(&bot, 0).await?; // main
    archive_ns(&bot, 4).await?; // Minecraft wiki
    archive_ns(&bot, 6).await?; // File
    archive_ns(&bot, 12).await?; // Help
    archive_ns(&bot, 110).await?; // Forum
    archive_ns(&bot, 10000).await?; // Minecraft Dungeons
    archive_ns(&bot, 10002).await?; // Minecraft Earth
    archive_ns(&bot, 10004).await?; // Minecraft Story Mode
    archive_ns(&bot, 10006).await?; // Minecraft Legends

    Ok(())
}

async fn archive_ns(bot: &MwBot, ns: u32) -> Result<()> {
    info!(ns, "archiving namespace");
    let mut gen = AllPages::new(ns)
        .filter_redirect(mwbot::generators::FilterRedirect::Nonredirects)
        .generate(&bot);
    while let Some(page) = gen.recv().await {
        match page {
            Result::Ok(page) => {
                archive_page(bot, page).await?;
            }
            Result::Err(err) => {
                error!(err = %err, "error");
            }
        }
    }
    Ok(())
}

async fn archive_page(bot: &MwBot, page: Page) -> Result<()> {
    info!(page = page.title(), "archiving page");
    let links: Vec<String> = serde_json::from_value(
        bot.api()
            .get_value(&[
                ("action", "parse"),
                ("page", page.title()),
                ("prop", "externallinks"),
            ])
            .await?["parse"]["externallinks"]
            .take(),
    )?;
    for url in links.into_iter() {
        if url.starts_with("https://web.archive.org/web/")
            || url.starts_with("http://web.archive.org/web/")
        {
            continue;
        }
        if !url.starts_with("https://") && !url.starts_with("http://") {
            continue;
        }
        if let Err(err) = archive_url(bot, page.title(), &url).await {
            error!(page = page.title(), url, err = %err, "error archiving page");
        }
    }
    Ok(())
}

const UA: LazyCell<String> = LazyCell::new(|| env::var("IA_UA").unwrap());
const API_ENDPOINT: LazyCell<String> = LazyCell::new(|| env::var("IA_API").unwrap());
const SPN_REGEX: LazyCell<Regex> = LazyCell::new(|| Regex::new(r"spn2-[a-z0-9-]*").unwrap());

async fn archive_url(bot: &MwBot, page: &str, url: &str) -> Result<()> {
    loop {
        let http = bot.api().http_client();
        let check_resp = http
            .get(Url::parse_with_params(
                "https://archive.org/wayback/available",
                &[("url", url)],
            )?)
            .send()
            .await?;
        if check_resp.status() == 429 {
            let delay = check_resp
                .headers()
                .get("Retry-After")
                .map(|s| s.to_str().ok())
                .unwrap_or(None)
                .unwrap_or_default()
                .parse::<u64>()
                .unwrap_or(30);
            debug!(delay, "IA API too many requests");
            // Too Many Requests
            time::sleep(time::Duration::from_secs(delay)).await;
            continue;
        }
        let check_resp = check_resp
            .error_for_status()?
            .json::<serde_json::Value>()
            .await?;
        if let Some(resp) = check_resp["archived_snapshots"].as_object() {
            if resp.contains_key("closest") {
                trace!(page, url, "skipping archived url");
                return Ok(());
            }
        }

        time::sleep(time::Duration::from_millis(
            (random::<u32>() % 120000) as u64,
        ))
        .await;

        info!(page, url, "archiving url");
        let archive_resp = http
            .post(Url::parse_with_params(
                format!("{}/save/", API_ENDPOINT.as_str()).as_str(),
                &[("url", url)],
            )?)
            .header("Content-Type", "application/x-www-form-urlencoded")
            .header("Accept", "text/html,application/xhtml+xml,application/xml")
            .header("User-Agent", UA.as_str())
            .form(&[("url", url)])
            .send()
            .await?
            .error_for_status()?
            .text()
            .await?;
        let spn = SPN_REGEX
            .find(&archive_resp)
            .ok_or_else(|| anyhow!("spn not found"))?
            .as_str();
        info!(url, spn, "archive request sent");

        let mut retries = 0;
        loop {
            time::sleep(time::Duration::from_secs(10)).await;
            let status_resp = http
                .get(format!("{}/save/status/{}", API_ENDPOINT.as_str(), spn))
                .header("Accept", "text/html,application/xhtml+xml,application/xml")
                .header("User-Agent", UA.as_str())
                .send()
                .await?
                .error_for_status()?
                .json::<serde_json::Value>()
                .await?;
            match status_resp["status"].as_str() {
                Some("pending") => {
                    trace!(page, url, "pending archive");
                }
                Some("success") => {
                    let status_code = status_resp["http_status"].as_u64().unwrap_or_default();
                    let duration = status_resp["duration_sec"].as_f64().unwrap_or_default();
                    let embeds = status_resp["counters"]["embeds"]
                        .as_u64()
                        .unwrap_or_default();
                    let outlinks = status_resp["counters"]["outlinks"]
                        .as_u64()
                        .unwrap_or_default();
                    info!(
                        page,
                        url,
                        status_code,
                        time = duration,
                        embeds,
                        outlinks,
                        "archived url successfully"
                    );
                    return Ok(());
                }
                None => bail!("status field not in /save/status/ response"),
                Some(&_) => bail!("unknown status in /save/status/ response"),
            }
            retries += 1;
            if retries > 60 {
                panic!("Too Many Requests")
            }
        }
    }
}
