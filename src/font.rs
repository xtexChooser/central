// SPDX-License-Identifier: GPL-3.0-or-later
// (C) Copyright 2023 Kunal Mehta <legoktm@debian.org>
enum Mode {
    RelativePlus,
    RelativeMinus,
    Absolute,
}

/// Implements the "rules for parsing a legacy font size"
/// https://html.spec.whatwg.org/multipage/rendering.html#phrasing-content-3
pub(crate) fn parse_legacy_font_size(input: &str) -> Option<&'static str> {
    // 1. Let input be the attribute's value.
    // 2. Let position be a pointer into input, initially pointing at the start of the string.
    // 3. Skip ASCII whitespace within input given position.
    //    (yes, we check Unicode whitespace, shhhh)
    let input = input.trim_start();
    // 4. If position is past the end of input, there is no presentational hint. Return.
    if input.is_empty() {
        return None;
    }
    let mut pos = 0;
    // 5. If the character at position is a U+002B PLUS SIGN character (+), then let mode be
    //    relative-plus, and advance position to the next character. Otherwise, if the character
    //    at position is a U+002D HYPHEN-MINUS character (-), then let mode be relative-minus,
    //    and advance position to the next character. Otherwise, let mode be absolute.
    let mode = match input.chars().next() {
        Some('+') => {
            pos += 1;
            Mode::RelativePlus
        }
        Some('-') => {
            pos += 1;
            Mode::RelativeMinus
        }
        _ => Mode::Absolute,
    };
    // 6. Collect a sequence of code points that are ASCII digits from input given position, and
    //    let the resulting sequence be digits.
    let mut digits = "".to_string();
    for c in input.chars().skip(pos) {
        if c.is_ascii_digit() {
            digits.push(c);
        } else {
            break;
        }
    }
    // 7. If digits is the empty string, there is no presentational hint. Return.
    if digits.is_empty() {
        return None;
    }
    // 8. Interpret digits as a base-ten integer. Let value be the resulting number.
    let mut value: isize = match digits.parse() {
        Ok(value) => value,
        Err(_) => {
            return None;
        }
    };
    // 9. If mode is relative-plus, then increment value by 3. If mode is relative-minus, then
    //    let value be the result of subtracting value from 3.
    match mode {
        Mode::RelativePlus => {
            value += 3;
        }
        Mode::RelativeMinus => {
            value = 3 - value;
        }
        Mode::Absolute => {}
    }
    // 10. If value is greater than 7, let it be 7.
    // 11. If value is less than 1, let it be 1.
    value = value.clamp(1, 7);
    // 12. Set 'font-size' to the keyword corresponding to the value of value according
    //     to the following table:
    let keyword = match value {
        1 => "x-small",
        2 => "small",
        3 => "medium",
        4 => "large",
        5 => "x-large",
        6 => "xx-large",
        7 => "xxx-large",
        val => unreachable!("got value of {}", val),
    };
    Some(keyword)
}

#[test]
fn test_parse_legacy_font_size() {
    assert_eq!(parse_legacy_font_size("2"), Some("small"));
    assert_eq!(parse_legacy_font_size("+2"), Some("x-large"));
    assert_eq!(parse_legacy_font_size("-8"), Some("x-small"));
    assert_eq!(parse_legacy_font_size("3px"), Some("medium"));
    assert_eq!(parse_legacy_font_size("ahhhh"), None);
    assert_eq!(parse_legacy_font_size("-"), None);
    assert_eq!(parse_legacy_font_size("-1"), Some("small"));
}

/// Implements the "rules for parsing a legacy color value"
/// https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-a-legacy-colour-value
pub(crate) fn parse_legacy_color_value(input: &str) -> Option<String> {
    // 1. Let input be the string being parsed.
    // 2. If input is the empty string, then return an error.
    if input.is_empty() {
        return None;
    }
    // 3. Strip leading and trailing ASCII whitespace from input.
    //    (yes, we check Unicode whitespace, shhhh)
    let input = input.trim();
    // 4. If input is an ASCII case-insensitive match for the string "transparent", then return
    //     an error.
    if input.to_ascii_lowercase() == "transparent" {
        return None;
    }
    // 5. If input is an ASCII case-insensitive match for one of the named colors, then return
    //     the simple color corresponding to that keyword.
    if crate::colors::NAMED_COLORS
        .contains(&input.to_ascii_lowercase().as_str())
    {
        return Some(input.to_ascii_lowercase());
    }
    // 6. If input's code point length is four, and the first character in input is U+0023 (#),
    //    and the last three characters of input are all ASCII hex digits, then:
    //    1. Let result be a simple color.
    //    ...
    //    5. Return result.
    if input.len() == 4 {
        let mut chars = input.chars();
        if chars.next() == Some('#') && chars.all(|c| c.is_ascii_hexdigit()) {
            return Some(input.to_string());
        }
    }
    // 7. Replace any code points greater than U+FFFF in input (i.e., any characters that are
    //     not in the basic multilingual plane) with the two-character string "00".
    //    (assume this doesn't apply to us)
    // 8. If input's code point length is greater than 128, truncate input, leaving only the
    //    first 128 characters.
    let mut input = input.to_string();
    if input.len() > 128 {
        input.truncate(128);
    }
    // 9. If the first character in input is a U+0023 NUMBER SIGN character (#), remove it.
    if input.starts_with('#') {
        input = input.strip_prefix('#').unwrap().to_string();
    }
    // 10. Replace any character in input that is not an ASCII hex digit with the character
    //     U+0030 DIGIT ZERO (0).
    let mut input: String = input
        .chars()
        .map(|c| if c.is_ascii_hexdigit() { c } else { '0' })
        .collect();
    // 11. While input's code point length is zero or not a multiple of three, append a
    //     U+0030 DIGIT ZERO (0) character to input.
    while input.len() % 3 != 0 {
        input.push('0');
    }
    // 12. Split input into three strings of equal code point length, to obtain three
    //     components. Let length be the code point length that all of those components
    //     have (one third the code point length of input).
    let mut length = input.len() / 3;
    let (first, remaining) = input.split_at(length);
    let (second, third) = remaining.split_at(length);
    let mut first = first.to_string();
    let mut second = second.to_string();
    let mut third = third.to_string();
    // 13. If length is greater than 8, then remove the leading length-8 characters in
    //     each component, and let length be 8.
    if length > 8 {
        let strip = length - 8;
        first = strip_chars(first, strip);
        assert_eq!(first.len(), 8);
        second = strip_chars(second, strip);
        third = strip_chars(third, strip);
        length = 8;
    }
    // 14. While length is greater than two and the first character in each component is
    //     a U+0030 DIGIT ZERO (0) character, remove that character and reduce length by one.
    while length > 2
        && first.starts_with('0')
        && second.starts_with('0')
        && third.starts_with('0')
    {
        first = strip_chars(first, 1);
        second = strip_chars(second, 1);
        third = strip_chars(third, 1);
        length -= 1;
    }
    // 15. If length is still greater than two, truncate each component, leaving only
    //     the first two characters in each.
    if length > 2 {
        first.truncate(2);
        second.truncate(2);
        third.truncate(2);
    }
    // 16. Let result be a simple color.
    Some(format!("#{first}{second}{third}"))
}

/// Strip the count number of characters from the left
fn strip_chars(input: String, count: usize) -> String {
    input.chars().skip(count).collect()
}

#[test]
fn test_parse_legacy_color_value() {
    assert_eq!(parse_legacy_color_value("transparent"), None);
    assert_eq!(parse_legacy_color_value("black"), Some("black".to_string()));
    assert_eq!(parse_legacy_color_value("#000"), Some("#000".to_string()));
    // User talk:Patmax23
    assert_eq!(
        parse_legacy_color_value("coal"),
        Some("#c0a000".to_string())
    );
    // FIXME: more test cases
}
