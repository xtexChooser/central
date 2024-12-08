use chrono::Utc;
use hiqlite::{params, Param};
use rauthy_common::is_hiqlite;
use rauthy_models::database::DB;
use std::env;
use std::ops::Sub;
use std::time::Duration;
use tracing::{debug, error};

/// Cleans up all Events that exceed the configured EVENT_CLEANUP_DAYS
pub async fn events_cleanup() {
    let mut interval = tokio::time::interval(Duration::from_secs(3600));

    let cleanup_days = env::var("EVENT_CLEANUP_DAYS")
        .unwrap_or_else(|_| "31".to_string())
        .parse::<u32>()
        .expect("Cannot parse EVENT_CLEANUP_DAYS to u32") as i64;

    loop {
        interval.tick().await;

        if !DB::client().is_leader_cache().await {
            debug!("Running HA mode without being the leader - skipping events_cleanup scheduler");
            continue;
        }

        debug!("Running events_cleanup scheduler");

        let threshold = Utc::now()
            .sub(chrono::Duration::days(cleanup_days))
            .timestamp_millis();

        if is_hiqlite() {
            let res = DB::client()
                .execute(
                    "DELETE FROM events WHERE timestamp < $1",
                    params!(threshold),
                )
                .await;

            match res {
                Ok(rows_affected) => {
                    debug!("Cleaned up {} expired events", rows_affected);
                }
                Err(err) => error!("Events cleanup error: {:?}", err),
            }
        } else {
            let res = sqlx::query!("DELETE FROM events WHERE timestamp < $1", threshold)
                .execute(DB::conn())
                .await;

            match res {
                Ok(r) => {
                    debug!("Cleaned up {} expired events", r.rows_affected());
                }
                Err(err) => error!("Events cleanup error: {:?}", err),
            }
        };
    }
}
