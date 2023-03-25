// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
use crate::{util, Options, Result, Summary};
use kuchiki::NodeRef;
use mwbot::parsoid::{image::HorizontalAlignment, prelude::*};

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
) -> Result<()> {
    if opts.center_gallery {
        // Only operate if all the children are galleries and none of them
        // have the `perrow` attribute set.
        let ok = center.children().all(|child| {
            // There is usually whitespace between the <center> and opening <gallery> tag in the HTML
            if let Some(text) = child.as_text() {
                return text.borrow().chars().all(|char| char.is_whitespace());
            }

            if let Some(gallery) = child.as_gallery() {
                !gallery
                    .attributes()
                    .unwrap_or_default()
                    .contains_key("perrow")
            } else {
                false
            }
        });
        if ok {
            // Iterate over it backwards so when we insert it ends up in order
            let children: Vec<_> = center.children().collect();
            for child in children.into_iter().rev() {
                // child is either a whitespace text node or a gallery tag, we'll
                // discard the whitespace and let Parsoid sort it out
                if let Some(gallery) = child.as_gallery() {
                    let class = gallery
                        .attributes()?
                        .get("class")
                        .unwrap_or(&"".to_string())
                        .to_string();
                    gallery.set_attribute(
                        "class",
                        format!("{class} center").trim(),
                    )?;
                    center.insert_after(&child);
                }
            }
            // Remove the center tag
            center.detach();
            summary.center += 1;
            return Ok(());
        }
    }

    if opts.center_image {
        // Only operate if there's a single child and it's an image
        let children: Vec<_> = center.children().collect();
        if children.len() == 1 {
            if let Some(image) = children[0].as_image() {
                let halign = image.horizontal_alignment();
                match halign {
                    // Already centered, do nothing
                    HorizontalAlignment::Center => {}
                    // Unspecified, center it
                    HorizontalAlignment::Unspecified => {
                        image.set_horizontal_alignment(
                            HorizontalAlignment::Center,
                        );
                    }
                    // Aligned in a different direction, can't auto fix it
                    _ => {
                        return Ok(());
                    }
                }
                // Move image out of center tag and remove it
                center.insert_after(&image);
                center.detach();
                summary.center += 1;
                return Ok(());
            }
        }
    }

    // If there's a child with an inline style with margin, it
    // will interfere with class="center" so we skip for now.
    if has_inline_margin(&center) {
        return Ok(());
    }
    // centering tables is disabled, if the center has descendants
    // that are tables, we skip.
    if !opts.center_tables && !center.select("table").is_empty() {
        return Ok(());
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
    summary.center += 1;
    Ok(())
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
