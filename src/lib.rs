// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
#![deny(clippy::all)]

pub mod api;
mod block;
mod center;
mod colors;
mod font;
mod legacy;
pub mod util;

use anyhow::Result;
use kuchikiki::NodeRef;
use lazy_static::lazy_static;
use mwbot::parsoid::map::IndexMap;
use mwbot::parsoid::prelude::*;
use regex::Regex;
use serde::{Deserialize, Deserializer};
use std::collections::HashSet;

#[derive(Copy, Clone)]
pub struct Options {
    /// Whether to fix <center> when it contains tables
    pub center_tables: bool,
    /// Whether to replace <strike> with <s>
    pub replace_strike: bool,
    /// Replace <tt>emoticon</tt> with {{mono|emoticon}}
    pub tt_emoticon: bool,
    /// Replace <center>[[File:Foo.jpg]]</center> with [[File:Foo.jpg|center]]
    pub center_image: bool,
    /// Replace <center><gallery/><center> with <gallery class="center"> when possible
    pub center_gallery: bool,
}

#[derive(Default)]
pub struct Summary {
    // Counts of tags fixed
    pub font: usize,
    pub center: usize,
    pub tt: usize,
    pub strike: usize,
    pub id: u32,
    pub remaining_lints: Vec<LintError>,
    pub no_change: bool,
    pub added_nowiki: bool,
    pub tags: HashSet<String>,
    pub assist: Option<String>,
}

impl Summary {
    pub fn edit_summary(&self) -> String {
        let mut counts = vec![];
        if self.font > 0 {
            counts.push(format!("<font> ({}x)", self.font));
        }
        if self.center > 0 {
            counts.push(format!("<center> ({}x)", self.center));
        }
        if self.tt > 0 {
            counts.push(format!("<tt> ({}x)", self.tt));
        }
        if self.strike > 0 {
            counts.push(format!("<strike> ({}x)", self.strike));
        }
        let assist = match &self.assist {
            Some(name) => format!(" assisted by [[User:{name}|{name}]]"),
            None => "".to_string(),
        };
        format!(
            "Bot{assist}: [[User:Legobot/Lint fixes|Fixing lint errors]], replacing [[mw:Help:Lint errors/obsolete-tag|obsolete HTML tags]]: {}",
            counts.join(", ")
        )
    }
}

#[derive(Deserialize, Debug)]
pub struct LintError {
    #[serde(rename = "type")]
    pub type_: String,
    #[serde(deserialize_with = "deserialize_dsr")]
    pub dsr: Dsr,
    pub params: LintErrorParams,
    #[serde(rename = "templateInfo")]
    pub template_info: Option<TemplateInfo>,
}

fn deserialize_dsr<'de, D>(input: D) -> Result<Dsr, D::Error>
where
    D: Deserializer<'de>,
{
    let array: [Option<u64>; 4] = Deserialize::deserialize(input)?;
    Ok(Dsr {
        // these two are required AFAICT
        start_offset: array[0].unwrap(),
        end_offset: array[1].unwrap(),
        start_tag_width: array[2],
        end_tag_width: array[3],
    })
}

#[derive(Deserialize, Debug)]
pub struct LintErrorParams {
    pub name: Option<String>,
    #[serde(default)]
    #[serde(rename = "inTable")]
    pub in_table: bool,
}

#[derive(Deserialize, Debug)]
pub struct TemplateInfo {
    #[serde(default)]
    #[serde(rename = "multiPartTemplateBlock")]
    pub multi_part_template_block: bool,
}

/// See dsr explanation at https://www.mediawiki.org/wiki/Parsoid/Internals/data-parsoid#Required_properties
#[derive(Debug)]
pub struct Dsr {
    pub start_offset: u64,
    pub end_offset: u64,
    pub start_tag_width: Option<u64>,
    pub end_tag_width: Option<u64>,
}

fn handle_strike(strike: &NodeRef) {
    let s = Wikicode::new_node("s");
    util::copy_attributes(strike, &s);
    util::copy_children(strike, &s);
    util::swap_nodes(strike, &s);
}

fn handle_tt(opts: &Options, tt: Wikinode, summary: &mut Summary) {
    // If the contents of tt is emoticon-looking, replace it with {{mono}}
    if opts.tt_emoticon {
        lazy_static! {
            static ref RE: Regex = Regex::new(r"^[:;]-?[DP/)]$").unwrap();
        }
        // Should just be a plain tt tag, no attributes (except "id")
        if let Some(element) = tt.as_element() {
            if element.attributes.borrow().map.len() <= 1
                && tt.children().all(|child| child.as_text().is_some())
            {
                let contents = tt.text_contents();
                if RE.is_match(&contents) {
                    // It's an emoticon, swap it with a template
                    tt.insert_after(
                        &Template::new(
                            "mono",
                            &IndexMap::from([("1".to_string(), contents)]),
                        )
                        .expect("invalid tt contents somehow??"),
                    );
                    tt.detach();
                    summary.tt += 1;
                    return;
                }
            }
        }
    }
    // Only replace tt if all the children are nowiki.
    // So: <tt><nowiki>...</nowiki></tt> -> <code><nowiki>...</nowiki></code>
    if !tt.children().all(|node| node.as_nowiki().is_some()) {
        return;
    }
    let code = Wikicode::new_node("code");
    util::copy_attributes(&tt, &code);
    util::copy_children(&tt, &code);
    util::swap_nodes(&tt, &code);
    summary.tt += 1;
}

pub fn delint_html(
    opts: &Options,
    html: ImmutableWikicode,
    summary: &mut Summary,
) -> Result<ImmutableWikicode> {
    let html = html.into_mutable();
    for font in html.select("font") {
        font::handle_font(font, summary);
        summary.font += 1;
    }
    if opts.replace_strike {
        for strike in html.select("strike") {
            handle_strike(&strike);
            summary.strike += 1;
        }
    }
    for tt in html.select("tt") {
        handle_tt(opts, tt, summary);
    }
    for center in html.select("center") {
        center::handle_center(opts, center, summary)?;
    }
    Ok(html.into_immutable())
}
