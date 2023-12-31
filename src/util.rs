// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
use kuchikiki::NodeRef;

/// Copy children from the old node to the new one
pub(crate) fn copy_children(old: &NodeRef, new: &NodeRef) {
    for child in old.children() {
        new.append(child);
    }
}

/// Replace the old node with the new one
pub(crate) fn swap_nodes(old: &NodeRef, new: &NodeRef) {
    old.insert_after(new.clone());
    old.detach();
}

/// Copy all attributes from the old node to the new one
pub(crate) fn copy_attributes(old: &NodeRef, new: &NodeRef) {
    let mut attrs = new.as_element().unwrap().attributes.borrow_mut();
    for (name, value) in &old.as_element().unwrap().attributes.borrow().map {
        attrs.insert(name.local.to_string(), value.value.to_string());
    }
}

pub fn escape(input: &str) -> String {
    input.replace('<', "&lt;")
}
