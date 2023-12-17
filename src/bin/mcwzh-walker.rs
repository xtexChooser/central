#![feature(let_chains)]
#![feature(lazy_cell)]

use std::{
    fs::{create_dir_all, File},
    path::Path,
};

use mwbot::generators::{categories::Categories, AllPages, Generator};
use xt_bot_wiki::{init_log, prelude::*};

#[tokio::main]
async fn main() -> Result<()> {
    init_log();
    create_dir_all("pub/mcwzh-walker")?;
    let bot = MwBot::from_path(&Path::new("config/mwbot-mcwzh.toml")).await?;

    let mut wtr: csv::Writer<File> =
        csv::Writer::from_writer(File::create("pub/mcwzh-walker/log.csv")?);

    let mut allpages = AllPages::new(0u32)
        .filter_redirect(mwbot::generators::FilterRedirect::All)
        .generate(&bot);
    while let Some(page) = allpages.recv().await {
        let page = page?;
        info!(page = page.title(), "checked page");
        // let wt = page.wikitext().await?;
        if let Some(redirect_target) = page.redirect_target().await? {
            // 命令重定向
            if redirect_target.title().starts_with("命令/") {
                if Categories::new(vec![page.title().to_string()])
                    .categories(vec!["Category:命令重定向".to_string()])
                    .generate(&bot)
                    .recv()
                    .await
                    .is_none()
                {
                    info!(page = page.title(), "missing command redirection category");
                    wtr.serialize(Log::MissingCommandRedirectionCat(page.title().to_string()))?;
                }
            }
        }
        if page.title().starts_with("命令/") {
            if !bot.page(&page.title()[2..]).unwrap().exists().await? {
                info!(page = page.title(), "missing command redirection page");
                wtr.serialize(Log::MissingCommandRedirectionCat(page.title().to_string()))?;
            }
        }
    }

    Ok(())
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub enum Log {
    MissingCommandRedirectionCat(String),
    MissingCommandRedirectionPage(String),
}
