#![feature(let_chains)]
#![feature(lazy_cell)]

use std::{
    cell::LazyCell,
    collections::HashMap,
    fs::{self, create_dir_all, read_to_string, File},
    io::Write,
    path::Path,
};

use chrono::NaiveDate;
use regex::Regex;
use similar::TextDiff;
use xt_bot_wiki::{mcw::McEdition, prelude::*};

#[tokio::main]
async fn main() -> Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(tracing_subscriber::EnvFilter::from_default_env())
        .init();
    create_dir_all("pub/mcwzh-croe-diff")?;
    let bot = MwBot::from_path(&Path::new("config/mwbot-mcwzh.toml")).await?;
    let http = bot.api().http_client();

    let en = http
        .execute(
            http.get("https://minecraft.wiki/rest.php/v1/page/Chronology%20of%20events")
                .build()?,
        )
        .await?
        .error_for_status()?
        .json::<serde_json::Value>()
        .await?;

    assert_eq!(en["content_model"].as_str(), Some("wikitext"));
    assert_eq!(en["title"].as_str(), Some("Chronology of events"));
    info!(
        rev_id = en["latest"]["id"].as_i64(),
        rev_time = en["latest"]["timestamp"].as_str(),
        "fetched en page"
    );
    let enwt = en["source"].as_str().unwrap().to_string();
    File::create("pub/mcwzh-croe-diff/en.txt")?.write_all(enwt.as_bytes())?;

    let zh = bot.page("大事记")?;
    info!("fetched zh page");
    let zhwt = zh.wikitext().await?;
    File::create("pub/mcwzh-croe-diff/zh.txt")?.write_all(zhwt.as_bytes())?;

    let en = parse_croe_en(enwt).await?;
    fs::write(
        "pub/mcwzh-croe-diff/en.json",
        serde_json::to_string_pretty(&en)?.as_bytes(),
    )?;

    let zh = parse_croe_zh(zhwt).await?;
    fs::write(
        "pub/mcwzh-croe-diff/zh.json",
        serde_json::to_string_pretty(&zh)?.as_bytes(),
    )?;

    {
        let mut summary = en.clone();
        drop_text(&mut summary);
        let mut wtr = csv::Writer::from_writer(File::create("pub/mcwzh-croe-diff/en.csv")?);
        for event in summary.events.iter() {
            wtr.serialize(event)?;
        }
    }

    {
        let mut summary: CroePage = zh.clone();
        drop_text(&mut summary);
        let mut wtr = csv::Writer::from_writer(File::create("pub/mcwzh-croe-diff/zh.csv")?);
        for event in summary.events.iter() {
            wtr.serialize(event)?;
        }
    }

    {
        let encsv = read_to_string("pub/mcwzh-croe-diff/en.csv")?;
        let zhcsv = read_to_string("pub/mcwzh-croe-diff/zh.csv")?;
        let diff = TextDiff::configure()
            .algorithm(similar::Algorithm::Patience)
            .diff_lines(&encsv, &zhcsv);

        let udiff = diff
            .unified_diff()
            .context_radius(5)
            .header("en", "zh")
            .to_string();
        fs::write("pub/mcwzh-croe-diff/diff.diff", udiff)?;
    }

    Ok(())
}

#[derive(Debug, Clone, Hash, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Default)]
struct CroePage {
    pub events: Vec<CroeEvent>,
}

#[derive(Debug, Clone, Hash, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize)]
struct CroeEvent {
    pub date: chrono::NaiveDate,
    pub edition: McEdition,
    pub text: String,
}

const MC_EDITION_KEYWORDS: LazyCell<HashMap<&str, McEdition>> = LazyCell::new(|| {
    HashMap::from([
        ("java", McEdition::JE),
        ("je", McEdition::JE),
        ("classic", McEdition::JE),
        ("bedrock", McEdition::BE),
        ("be", McEdition::BE),
        ("基岩版", McEdition::BE),
        ("education", McEdition::EDU),
        ("教育版", McEdition::EDU),
        ("chinese", McEdition::CHN),
        ("中国版", McEdition::CHN),
        ("dungeons", McEdition::MCD),
        ("legends", McEdition::MCL),
        ("legacy console", McEdition::LegacyConsole),
        ("原主机版", McEdition::LegacyConsole),
        ("pocket edition", McEdition::PE),
        ("携带版", McEdition::PE),
        ("earth", McEdition::Earth),
        ("story mode", McEdition::StoryMode),
    ])
});

fn match_mc_edition(text: &str) -> McEdition {
    let text = text.to_lowercase();
    for (kw, ed) in MC_EDITION_KEYWORDS.iter() {
        if text.contains(kw) {
            return ed.to_owned();
        }
    }
    McEdition::Unknown
}

const RE_TITLE: &str = r"^=+\s*([^=]+)\s*=+$";
const RE_NOWIKI: &str = r"<nowiki\s*(\/>|>.*<\/nowiki>)";

const EN_MONTHS: [&str; 12] = [
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
];

async fn parse_croe_en(wt: String) -> Result<CroePage> {
    let mut page = CroePage::default();

    let re_title = Regex::new(RE_TITLE)?;
    let re_year_title = Regex::new(r"^==\s*([1-9][0-9][0-9][0-9])\s*==$")?;
    let re_month_title = Regex::new(
        r"^===\s*(January|February|March|April|May|June|July|August|September|October|November|December)\s*([1-9][0-9][0-9][0-9])\s*===$",
    )?;
    let re_event_line = Regex::new(r"^\*\s*'*([1-9][0-9]*|\?)'*\s*[–—\-]\s*(.*)$")?;
    let re_nowiki = Regex::new(RE_NOWIKI)?;

    let mut year: Option<u32> = None;
    let mut month: Option<u32> = None;

    for ln in wt.lines() {
        let ln = ln.trim();
        if ln.is_empty() {
            continue;
        }
        let ln = re_nowiki.replace_all(ln, "").to_string();
        if let Some(cap) = re_year_title.captures(&ln) {
            let (_, [new_year]) = cap.extract();
            let new_year = new_year.parse()?;
            info!(new_year, "en parse year line");
            year = Some(new_year);
            month = None;
        } else if let Some(cap) = re_month_title.captures(&ln) {
            let (_, [new_month, _]) = cap.extract();
            let new_month = (EN_MONTHS.iter().position(|s| s == &new_month).unwrap() as u32) + 1;
            info!(new_month, "en parse month line");
            month = Some(new_month);
        } else if let Some(cap) = re_title.captures(&ln) {
            let (_, [title]) = cap.extract();
            info!(title, "en parse unknown title line");
            year = None;
            month = None;
        } else if let Some(year) = year
            && let Some(month) = month
        {
            if let Some(cap) = re_event_line.captures(&ln) {
                let (_, [day, text]) = cap.extract();
                let day = if day == "?" { 1 } else { day.parse::<u32>()? };
                let edition = match_mc_edition(&text);
                info!(
                    year,
                    month,
                    day,
                    text,
                    edition = format!("{:?}", edition),
                    "en parse event line"
                );
                page.events.push(CroeEvent {
                    date: NaiveDate::from_ymd_opt(year as i32, month, day).unwrap(),
                    edition,
                    text: text.to_string(),
                })
            } else {
                // error!(year, month, line = ln, "could not parse event line");
                page.events.last_mut().unwrap().text.push_str(&ln);
            }
        }
    }
    Ok(page)
}

async fn parse_croe_zh(wt: String) -> Result<CroePage> {
    let mut page = CroePage::default();

    let re_title = Regex::new(RE_TITLE)?;
    let re_year_title = Regex::new(r"^==\s*([1-9][0-9][0-9][0-9])\s*年\s*==$")?;
    let re_month_title = Regex::new(r"^===\s*([1-9][0-9]*)\s*月\s*===$")?;
    let re_event_line = Regex::new(r"^\*\s*'*([1-9][0-9]*|\?)\s*(日|)'*\s*[–—\-]\s*(.*)$")?;
    let re_nowiki = Regex::new(RE_NOWIKI)?;

    let mut year: Option<u32> = None;
    let mut month: Option<u32> = None;

    for ln in wt.lines() {
        let ln = ln.trim();
        if ln.is_empty() {
            continue;
        }
        let ln = re_nowiki.replace_all(ln, "").to_string();
        if let Some(cap) = re_year_title.captures(&ln) {
            let (_, [new_year]) = cap.extract();
            let new_year = new_year.parse()?;
            info!(new_year, "zh parse year line");
            year = Some(new_year);
            month = None;
        } else if let Some(cap) = re_month_title.captures(&ln) {
            let (_, [new_month]) = cap.extract();
            let new_month = new_month.parse()?;
            info!(new_month, "zh parse month line");
            month = Some(new_month);
        } else if let Some(cap) = re_title.captures(&ln) {
            let (_, [title]) = cap.extract();
            info!(title, "zh parse unknown title line");
            year = None;
            month = None;
        } else if let Some(year) = year
            && let Some(month) = month
        {
            if let Some(cap) = re_event_line.captures(&ln) {
                let (_, [day, _, text]) = cap.extract();
                let day = if day == "?" || day == "？" {
                    1
                } else {
                    day.parse::<u32>()?
                };
                let edition = match_mc_edition(&text);
                info!(
                    year,
                    month,
                    day,
                    text,
                    edition = format!("{:?}", edition),
                    "zh parse event line"
                );
                page.events.push(CroeEvent {
                    date: NaiveDate::from_ymd_opt(year as i32, month, day).unwrap(),
                    edition,
                    text: text.to_string(),
                })
            } else {
                // error!(year, month, line = ln, "could not parse event line");
                page.events.last_mut().unwrap().text.push_str(&ln);
            }
        }
    }
    Ok(page)
}

fn drop_text(page: &mut CroePage) {
    for event in page.events.iter_mut() {
        event.text = String::new();
    }
}
