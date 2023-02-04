// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

mod block;
mod center;
mod colors;
mod font;
mod legacy;
mod util;

use anyhow::Result;
use kuchiki::NodeRef;
use mwbot::parsoid::prelude::*;
use mwbot::Bot;
use serde::Deserialize;
use std::collections::HashSet;

pub struct Options {
    /// Whether to fix <center> when it contains tables
    pub center_tables: bool,
}

#[derive(Default)]
pub struct Summary {
    pub font: bool,
    pub center: bool,
    pub tt: bool,
    pub strike: bool,
    pub id: u32,
    pub remaining_lints: Vec<String>,
    pub no_change: bool,
    pub added_nowiki: bool,
    pub tags: HashSet<String>,
}

#[derive(Deserialize, Debug)]
pub struct LintError {
    #[serde(rename = "type")]
    pub type_: String,
}

pub async fn lint_errors(
    bot: &Bot,
    title: &str,
    wikitext: &str,
) -> Result<Vec<LintError>> {
    let req = bot.api().http_client().post(
        format!("https://en.wikipedia.org/api/rest_v1/transform/wikitext/to/lint/{}", urlencoding::encode(title)
    )).form(&[("wikitext", wikitext)])
            .build()?;
    let resp = bot.api().http_client().execute(req).await?.json().await?;
    Ok(resp)
}

fn handle_strike(strike: &NodeRef) {
    let s = Wikicode::new_node("s");
    util::copy_attributes(strike, &s);
    util::copy_children(strike, &s);
    util::swap_nodes(strike, &s);
}

fn handle_tt(tt: Wikinode, summary: &mut Summary) {
    // Only replace tt if all the children are nowiki.
    // So: <tt><nowiki>...</nowiki></tt> -> <code><nowiki>...</nowiki></code>
    if !tt.children().all(|node| node.as_nowiki().is_some()) {
        return;
    }
    let code = Wikicode::new_node("code");
    util::copy_attributes(&tt, &code);
    util::copy_children(&tt, &code);
    util::swap_nodes(&tt, &code);
    summary.tt = true;
}

pub fn delint_html(
    opts: &Options,
    html: ImmutableWikicode,
    summary: &mut Summary,
) -> Result<ImmutableWikicode> {
    let html = html.into_mutable();
    for font in html.select("font") {
        println!("found <font>");
        font::handle_font(font, summary);
        summary.font = true;
    }
    for strike in html.select("strike") {
        println!("found <strike>");
        handle_strike(&strike);
        summary.strike = true;
    }
    for tt in html.select("tt") {
        println!("found <tt>");
        handle_tt(tt, summary);
    }
    for center in html.select("center") {
        println!("found <center>");
        center::handle_center(opts, center, summary);
        summary.center = true;
    }
    Ok(html.into_immutable())
}
