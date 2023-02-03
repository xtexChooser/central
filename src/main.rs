// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
mod block;
mod colors;
mod font;

use anyhow::Result;
use kuchiki::NodeRef;
use mwbot::parsoid::prelude::*;
use mwbot::{Bot, Page};
use serde::Deserialize;
use std::collections::{HashMap, HashSet};
use std::ops::Deref;
use tokio::fs;

#[derive(Default)]
struct Summary {
    font: bool,
    center: bool,
    tt: bool,
    strike: bool,
    id: u32,
    remaining_lints: Vec<String>,
    no_change: bool,
    tags: HashSet<String>,
}

#[tokio::main]
async fn main() -> Result<()> {
    mwbot::init_logging();
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
            let summary = process_page(&bot, &page).await?;
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
    fs::write("public_html/demo1/index.html", index.join("\n")).await?;

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

async fn process_page(bot: &Bot, page: &Page) -> Result<Summary> {
    println!("Checking {}...", page.title());
    let page_id = page.id().await?.expect("page doesn't exist");
    let original_html = page.html().await?;
    let mut summary = Summary {
        id: page_id,
        ..Default::default()
    };
    let html = handle_page(original_html.clone(), &mut summary)?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
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
        format!("public_html/demo1/html/{page_id}_original.html"),
        hack_stylesheet(original_html).html(),
    )
    .await?;
    fs::write(
        format!("public_html/demo1/html/{page_id}_modified.html"),
        hack_stylesheet(html).html(),
    )
    .await?;
    let remaining_lints = if summary.remaining_lints.is_empty() {
        "none".to_string()
    } else {
        summary.remaining_lints.join(", ")
    };
    let formatted = include_str!("diff.html")
        .replace("{diff}", &html_diff(bot, &original, &new_text).await?)
        .replace("{title}", page.title())
        .replace("{pageid}", &page_id.to_string())
        .replace("{lints}", &remaining_lints);

    fs::write(format!("public_html/demo1/{page_id}_diff.html"), formatted)
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

#[derive(Deserialize, Debug)]
struct LintError {
    #[serde(rename = "type")]
    type_: String,
}

async fn lint_errors(
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

/// Copy children from the old node to the new one
fn copy_children(old: &NodeRef, new: &NodeRef) {
    for child in old.children() {
        new.append(child);
    }
}

/// Replace the old node with the new one
fn swap_nodes(old: &NodeRef, new: &NodeRef) {
    old.insert_after(new.clone());
    old.detach();
}

/// Copy all attributes from the old node to the new one
fn copy_attributes(old: &NodeRef, new: &NodeRef) {
    let mut attrs = new.as_element().unwrap().attributes.borrow_mut();
    for (name, value) in &old.as_element().unwrap().attributes.borrow().map {
        attrs.insert(name.local.to_string(), value.value.to_string());
    }
}

/// If we need to wrap the text of links with the color, then we need to know
/// which color source we should use. If it's set via inline style, that takes
/// preference, before using the color attribute.
#[derive(Debug, Eq, PartialEq)]
enum ColorSource {
    InlineStyle(String),
    Attribute(String),
    None,
}

impl ColorSource {
    fn set_from_attr(&mut self, value: String) {
        match self {
            Self::InlineStyle(_) => {
                // Nothing, inline style takes priority
            }
            Self::Attribute(_) => {
                *self = Self::Attribute(value);
            }
            Self::None => {
                *self = Self::Attribute(value);
            }
        }
    }

    fn set_from_inline_style(&mut self, value: String) {
        *self = Self::InlineStyle(value);
    }

    fn into_style(self) -> Option<String> {
        match self {
            Self::InlineStyle(style) => Some(style),
            Self::Attribute(color) => Some(format!("color: {color};")),
            Self::None => None,
        }
    }
}

fn handle_font(font: Wikinode, summary: &mut Summary) {
    // First we need to figure out what tag we're going to replace it with.
    // If there are any block elements inside the <font> tags, we're going to
    // use a <div>. Otherwise we'll go with a <span>.
    let has_block = font.descendants().any(|node| {
        if let Some(element) = node.as_element() {
            block::BLOCK_ELEMENTS
                .contains(&element.name.local.to_string().as_str())
        } else {
            false
        }
    });
    let replacement_tag = if has_block {
        summary.tags.insert("block element inside font".to_string());
        "div"
    } else {
        "span"
    };
    let replacement = Wikicode::new_node(replacement_tag);
    let mut style = vec![];
    let mut color = ColorSource::None;
    // Copy attributes from <font> to <span/div>
    for (name, value) in &font.as_element().unwrap().attributes.borrow().map {
        let attr = name.local.to_string();
        match attr.as_str() {
            "color" | "colour" => {
                if let Some(parsed) =
                    font::parse_legacy_color_value(&value.value)
                {
                    style.push(format!("color: {};", &parsed));
                    color.set_from_attr(parsed);
                }
            }
            "face" => {
                // XXX: in theory some should be quoted, but it doesn't seem necessary
                style.push(format!("font-family: {};", &value.value));
            }
            "size" => {
                if let Some(font_size) =
                    font::parse_legacy_font_size(&value.value)
                {
                    style.push(format!("font-size: {font_size};"));
                }
            }
            // style needs to be merged in with our new styles
            "style" => {
                style.push(value.value.to_string());
                if value.value.contains("color:") {
                    color.set_from_inline_style(value.value.to_string());
                }
            }
            other => {
                // Pass it back as an attribute on <span>
                replacement
                    .as_element()
                    .unwrap()
                    .attributes
                    .borrow_mut()
                    .insert(other, value.value.to_string());
            }
        }
    }
    if !style.is_empty() {
        replacement
            .as_element()
            .unwrap()
            .attributes
            .borrow_mut()
            .insert("style", style.join(" "));
    }
    copy_children(&font, &replacement);
    swap_nodes(&font, &replacement);

    // If the <font> tag contained a color directive, it no longer applies to children
    // of any <a> tag because of https://www.mediawiki.org/wiki/Help:Lint_errors/tidy-font-bug
    // So we wrap all the children of the <a> tag with a new span with just the color directive.
    if let Some(style_color) = color.into_style() {
        // We find all descendant <a> tags
        for link in replacement.filter_links() {
            // We need to make sure there's not something else
            // coloring this node, e.g. <color><foo><color><a></a></color></foo></color>
            match find_coloring_node(&link) {
                Some(node) => {
                    if &node != replacement.deref() {
                        continue;
                    }
                }
                None => {
                    continue;
                }
            };
            for child in link.children() {
                if is_colored_node(&child) {
                    // Already has a color set, so we don't need to wrap
                    continue;
                }
                let inner = Wikicode::new_node("span");
                // We only need to style it with color
                inner
                    .as_element()
                    .unwrap()
                    .attributes
                    .borrow_mut()
                    .insert("style", style_color.to_string());
                child.insert_after(&inner);
                inner.append(&child);
                println!("{}", inner.to_string());
                link.append(&inner);
                println!("{}", link.to_string());
                summary.tags.insert("link inside font".to_string());
            }
        }
    }
}

fn find_coloring_node(node: &NodeRef) -> Option<NodeRef> {
    let mut node = node.clone();
    loop {
        node = node.parent()?;
        if is_colored_node(&node) {
            return Some(node);
        }
    }
}

fn is_colored_node(node: &NodeRef) -> bool {
    if let Some(element) = node.as_element() {
        // If it is <font color="...">
        if element.name.local.to_string() == "font"
            && element.attributes.borrow().contains("color")
        {
            return true;
        }
        // Otherwise check if there is a color inline style
        if let Some(style) = element.attributes.borrow().get("style") {
            style.contains("color:")
        } else {
            false
        }
    } else {
        false
    }
}

fn handle_strike(strike: Wikinode) {
    let s = Wikicode::new_node("s");
    copy_attributes(&strike, &s);
    copy_children(&strike, &s);
    swap_nodes(&strike, &s);
}

fn handle_tt(tt: Wikinode, summary: &mut Summary) {
    // Only replace tt if all the children are nowiki.
    // So: <tt><nowiki>...</nowiki></tt> -> <code><nowiki>...</nowiki></code>
    if !tt.children().all(|node| node.as_nowiki().is_some()) {
        return;
    }
    let code = Wikicode::new_node("code");
    copy_attributes(&tt, &code);
    copy_children(&tt, &code);
    swap_nodes(&tt, &code);
    summary.tt = true;
}

/// Add the specified class to a node, if it
/// doesn't have it already
fn add_class(node: &NodeRef, desired: &str) {
    if let Some(element) = node.as_element() {
        let class = element
            .attributes
            .borrow()
            .get("class")
            .unwrap_or("")
            .to_string();
        let mut sp: Vec<_> = class.split(' ').map(|s| s.to_string()).collect();
        if !sp.contains(&desired.to_string()) {
            sp.push(desired.to_string());
            let class = sp.join(" ").trim().to_string();
            element.attributes.borrow_mut().insert("class", class);
        }
    }
}

fn handle_center(center: Wikinode, summary: &mut Summary) {
    let div = Wikicode::new_node("div");
    copy_children(&center, &div);
    copy_attributes(&center, &div);
    add_class(&div, "center");
    // Per [[Help:TABLECENTER]] we need to assign class="center" to any tables
    for table in div.select("table") {
        add_class(&table, "center");
        summary.tags.insert("table inside center".to_string());
    }
    swap_nodes(&center, &div);
}

fn handle_page(
    html: ImmutableWikicode,
    summary: &mut Summary,
) -> Result<ImmutableWikicode> {
    let html = html.into_mutable();
    for font in html.select("font") {
        println!("found <font>");
        handle_font(font, summary);
        summary.font = true;
    }
    for strike in html.select("strike") {
        println!("found <strike>");
        handle_strike(strike);
        summary.strike = true;
    }
    for tt in html.select("tt") {
        println!("found <tt>");
        handle_tt(tt, summary);
    }
    for center in html.select("center") {
        println!("found <center>");
        handle_center(center, summary);
        summary.center = true;
    }
    Ok(html.into_immutable())
}

#[test]
fn test_colorsource() {
    let mut color = ColorSource::None;
    color.set_from_attr("foo".to_string());
    assert_eq!(color, ColorSource::Attribute("foo".to_string()));
    color.set_from_inline_style("bar".to_string());
    assert_eq!(color, ColorSource::InlineStyle("bar".to_string()));
    color.set_from_attr("foo".to_string()); // Has no effect
    assert_eq!(color, ColorSource::InlineStyle("bar".to_string()));
}

fn hack_stylesheet(html: ImmutableWikicode) -> ImmutableWikicode {
    // <link href="https://en.wikipedia.org/w/load.php?modules=mediawiki.diff.styles|skins.vector.styles.legacy&only=styles" rel="stylesheet">
    let html = html.into_mutable();
    let link = Wikicode::new_node("link");
    let attribs = &link.as_element().unwrap().attributes;
    attribs.borrow_mut().insert("href", "https://en.wikipedia.org/w/load.php?modules=mediawiki.diff.styles|skins.vector.styles.legacy&only=styles".to_string());
    attribs.borrow_mut().insert("rel", "stylesheet".to_string());
    html.select_first("head").unwrap().append(&link);
    html.into_immutable()
}
