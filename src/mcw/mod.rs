pub use crate::prelude::*;

#[derive(
    Debug, Clone, Copy, Hash, Serialize, Deserialize, Default, PartialEq, Eq, PartialOrd, Ord,
)]
pub enum McEdition {
    #[default]
    Unknown,
    JE,
    BE,
    EDU,
    CHN,
    MCD,
    MCL,
    LegacyConsole,
    PE,
    Earth,
    StoryMode,
}
