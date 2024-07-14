//! ## Stats of fixed tags
//!
//! Generate a stats.tsv file with:
//! ```
//! echo 'select rev_id, comment_text from revision_userindex join actor_revision on rev_actor = actor_id join comment_revision on rev_comment_id = comment_id join change_tag on rev_id = ct_rev_id join change_tag_def on ct_tag_id = ctd_id where actor_name = "<USERNAME>" and ctd_name = "fixed lint errors"' | sql enwiki > foo.csv
//! ```

use regex::Regex;
use std::collections::HashMap;
use std::fs::File;
use std::io::{self, BufRead};

fn main() -> io::Result<()> {
    let path = "stats.tsv";
    let file = File::open(path)?;
    let reader = io::BufReader::new(file);

    let mut tag_counts = HashMap::new();
    let mut lines_processed = 0;

    let re = Regex::new(r"<(\w+)>\s*\((\d+)x\)").unwrap();

    for line in reader.lines().skip(1) {
        let line = line?;
        lines_processed += 1;

        for cap in re.captures_iter(&line) {
            let tag = cap[1].to_string();
            let count: i32 = cap[2].parse().unwrap();
            *tag_counts.entry(tag).or_insert(0) += count;
        }
    }

    println!("Summary of fixed HTML tags:");
    for (tag, count) in tag_counts.iter() {
        println!("{}: {}", tag, count);
    }

    let total_tags: i32 = tag_counts.values().sum();
    println!("\nTotal tags fixed: {}", total_tags);
    println!("Lines processed: {}", lines_processed);

    Ok(())
}
