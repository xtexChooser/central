// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
use crate::{util, Options, Summary};
use kuchiki::NodeRef;
use mwbot::parsoid::prelude::*;

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

pub(crate) fn handle_center(
    opts: &Options,
    center: Wikinode,
    summary: &mut Summary,
) {
    // If there's a child with an inline style with margin, it
    // will interfere with class="center" so we skip for now.
    if has_inline_margin(&center) {
        return;
    }
    // centering tables is disabled, if the center has descendants
    // that are tables, we skip.
    if !opts.center_tables && !center.select("table").is_empty() {
        return;
    }
    let div = Wikicode::new_node("div");
    util::copy_children(&center, &div);
    util::copy_attributes(&center, &div);
    add_class(&div, "center");
    // Per [[Help:TABLECENTER]] we need to assign class="center" to any tables
    for table in div.select("table") {
        // TODO: We should use text-align: left; margin-left: auto; margin-right: auto
        // instead of class="center"
        // TODO: We should also detect transcluded or generated content that
        // we can't actually modify
        add_class(&table, "center");
        summary.tags.insert("table inside center".to_string());
    }
    util::swap_nodes(&center, &div);
}

fn has_inline_margin(node: &NodeRef) -> bool {
    if let Some(element) = node.as_element() {
        if let Some(style) = element.attributes.borrow().get("style") {
            if style.contains("margin") {
                return true;
            }
        }
    }
    node.children().any(|node| has_inline_margin(&node))
}
