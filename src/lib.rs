pub mod prelude;

pub mod mcw;

pub fn init_log() {
    use tracing::level_filters::LevelFilter;
    tracing_subscriber::fmt()
        .with_env_filter(
            tracing_subscriber::EnvFilter::builder()
                .with_default_directive(LevelFilter::INFO.into())
                .with_env_var("RUST_LOG")
                .from_env_lossy(),
        )
        .init();
}
