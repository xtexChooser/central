// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
mod colors;
mod font;

use anyhow::Result;
use kuchiki::NodeRef;
use mwbot::parsoid::prelude::*;
use mwbot::{Bot, Page};
use serde::Deserialize;
use std::collections::{HashMap, HashSet};
use tokio::fs;

#[tokio::main]
async fn main() -> Result<()> {
    let bot = Bot::from_default_config().await?;
    let pages = bot
        .api()
        .get_value(HashMap::from([
            ("action", "query"),
            ("list", "linterrors"),
            ("lntcategories", "obsolete-tag"),
            ("lntlimit", "max"),
        ]))
        .await?;
    let mut seen = HashSet::new();
    for error in pages["query"]["linterrors"].as_array().unwrap() {
        let title = error["title"].as_str().unwrap().to_string();
        if !seen.contains(&title) {
            // TODO: should we check and skip on templateInfo?
            let page = bot.page(&title)?;
            process_page(&bot, page).await?;
            seen.insert(title);
        }
    }
    Ok(())
}

async fn process_page(bot: &Bot, page: Page) -> Result<()> {
    println!("Checking {}...", page.title());
    let page_id = page.id().await?.expect("page doesn't exist");
    let original_html = page.html().await?;
    let html = handle_page(original_html.clone())?;
    let original = page.wikitext().await?;
    let new_text = bot.parsoid().transform_to_wikitext(&html).await?;
    let remaining = lint_errors(bot, page.title(), &new_text).await?;
    if !remaining.is_empty() {
        let remaining: Vec<_> =
            remaining.into_iter().map(|l| l.type_).collect();
        println!(
            "{} still has some lint errors, skipping ({})",
            page.title(),
            remaining.join(", ")
        );
        return Ok(());
    }
    if original == new_text {
        // In theory this should still trip lint errors, but double check just in case
        println!("No changes to {}, skipping", page.title());
        return Ok(());
    }
    fs::write(
        format!("output/html/{page_id}_original.html"),
        original_html.html(),
    )
    .await?;
    fs::write(format!("output/html/{page_id}_modified.html"), html.html())
        .await?;
    let formatted = include_str!("diff.html")
        .replace("{diff}", &html_diff(bot, &original, &new_text).await?)
        .replace("{title}", page.title())
        .replace("{pageid}", &page_id.to_string());

    fs::write(format!("output/{page_id}_diff.html"), formatted).await?;
    Ok(())
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

fn handle_font(font: Wikinode) {
    let span = Wikicode::new_node("span");
    let mut style = vec![];
    let mut color = None;
    // Copy attributes from <font> to <span>
    for (name, value) in &font.as_element().unwrap().attributes.borrow().map {
        let attr = name.local.to_string();
        match attr.as_str() {
            "color" => {
                if let Some(parsed) =
                    font::parse_legacy_color_value(&value.value)
                {
                    style.push(format!("color: {};", &parsed));
                    color = Some(parsed);
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
                    style.push(format!("font-size: {};", font_size));
                }
            }
            // style needs to be merged in with our new styles
            "style" => style.push(value.value.to_string()),
            other => {
                // Pass it back as an attribute on <span>
                span.as_element()
                    .unwrap()
                    .attributes
                    .borrow_mut()
                    .insert(other, value.value.to_string());
            }
        }
    }
    if !style.is_empty() {
        span.as_element()
            .unwrap()
            .attributes
            .borrow_mut()
            .insert("style", style.join(" "));
    }
    copy_children(&font, &span);
    swap_nodes(&font, &span);

    // If the <font> tag contained a color directive, it no longer applies to children
    // of any <a> tag because of https://www.mediawiki.org/wiki/Help:Lint_errors/tidy-font-bug
    // So we wrap all the children of the <a> tag with a new span with just the color directive.
    // TODO: what if the color is in a style directive, e.g. [[User:Universe=atom]]?
    if let Some(color) = color {
        for child in span.children() {
            if let Some(element) = child.as_element() {
                if element.name.local.to_string() == "a" {
                    for grandchild in child.children() {
                        let inner = Wikicode::new_node("span");
                        // We only need to style it with color
                        inner
                            .as_element()
                            .unwrap()
                            .attributes
                            .borrow_mut()
                            .insert("style", format!("color: {color};"));
                        grandchild.insert_after(&inner);
                        inner.append(&grandchild);
                        println!("{}", inner.to_string());
                        child.append(&inner);
                        println!("{}", child.to_string());
                    }
                }
            }
        }
    }
}

fn handle_strike(strike: Wikinode) {
    let s = Wikicode::new_node("s");
    copy_attributes(&strike, &s);
    copy_children(&strike, &s);
    swap_nodes(&strike, &s);
}

fn handle_tt(tt: Wikinode) {
    // Only replace tt if all the children are nowiki.
    // So: <tt><nowiki>...</nowiki></tt> -> <code><nowiki>...</nowiki></tt>
    if !tt.children().all(|node| node.as_nowiki().is_some()) {
        return;
    }
    let code = Wikicode::new_node("code");
    copy_attributes(&tt, &code);
    copy_children(&tt, &code);
    swap_nodes(&tt, &code);
}

fn handle_center(center: Wikinode) {
    let div = Wikicode::new_node("div");
    let mut style = vec!["text-align: center".to_string()];
    // Copy attributes from <center> to <div>
    for (name, value) in &center.as_element().unwrap().attributes.borrow().map {
        let attr = name.local.to_string();
        match attr.as_str() {
            // style needs to be merged in with our new styles
            "style" => style.push(value.value.to_string()),
            other => {
                // Pass it back as an attribute on <span>
                div.as_element()
                    .unwrap()
                    .attributes
                    .borrow_mut()
                    .insert(other, value.value.to_string());
            }
        }
    }
    if !style.is_empty() {
        div.as_element()
            .unwrap()
            .attributes
            .borrow_mut()
            .insert("style", style.join(" "));
    }
    copy_children(&center, &div);
    swap_nodes(&center, &div);
}

fn handle_page(html: ImmutableWikicode) -> Result<ImmutableWikicode> {
    let html = html.into_mutable();
    for font in html.select("font") {
        println!("found <font>");
        handle_font(font);
    }
    for strike in html.select("strike") {
        println!("found <strike>");
        handle_strike(strike);
    }
    for tt in html.select("tt") {
        println!("found <tt>");
        handle_tt(tt);
    }
    for center in html.select("center") {
        println!("found <center>");
        handle_center(center);
    }
    Ok(html.into_immutable())
}
