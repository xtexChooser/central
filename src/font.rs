// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
use crate::{block, legacy, util, Summary};
use kuchikiki::NodeRef;
use mwbot::parsoid::prelude::*;
use std::ops::Deref;

pub(crate) fn handle_font(font: Wikinode, summary: &mut Summary) {
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
                    legacy::parse_legacy_color_value(&value.value)
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
                    legacy::parse_legacy_font_size(&value.value)
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
    util::copy_children(&font, &replacement);
    util::swap_nodes(&font, &replacement);

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
                link.append(&inner);
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
