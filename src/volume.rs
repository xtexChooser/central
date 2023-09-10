use std::{cmp, collections::HashMap};

use anyhow::{Context, Result};
use podman_api::{
    api::Volumes,
    models::VolumeInspect,
    opts::{VolumeCreateOpts, VolumeCreateOptsBuilder, VolumeListFilter, VolumeListOpts},
};
use serde::{Deserialize, Serialize};
use tracing::info;

use crate::{
    constant::{LABEL_NO_PURGE, LABEL_NO_PURGE_VAL},
    direct_into_build,
};

#[derive(Debug, PartialEq, Eq, Serialize, Deserialize, Clone, Default)]
pub struct VolumeResources {
    #[serde(default)]
    pub created: Vec<VolumeCreated>,
    #[serde(default)]
    pub removed: Vec<VolumeRemoved>,
}

impl VolumeResources {
    pub async fn apply(&self, api: &Volumes) -> Result<()> {
        for created in &self.created {
            let remote_vol = api.get(&created.name);
            if remote_vol.exists().await? {
                if created == &remote_vol.inspect().await? {
                    continue;
                } else {
                    remote_vol.delete().await?;
                    info!(
                        name = created.name,
                        "deleted exists volume for not matching"
                    )
                }
            }
            let resp = api
                .create(&created.clone().into())
                .await
                .with_context(|| format!("target volume: {}", &created.name))?;
            info!(
                name = created.name,
                response = serde_json::to_string(&resp)?,
                "created volume"
            );
        }
        for removed in &self.removed {
            let remote_vol = api.get(&removed.name);
            if remote_vol.exists().await? {
                let force = removed.force.unwrap_or(false);
                if force {
                    remote_vol.remove().await?;
                } else {
                    remote_vol.delete().await?;
                }
                info!(name = removed.name, force, "deleted exists volume");
            }
        }
        Ok(())
    }

    pub async fn purge(&self, api: &Volumes) -> Result<()> {
        let volume = api
            .list(
                &VolumeListOpts::builder()
                    .filter([VolumeListFilter::NoLabelKeyVal(
                        LABEL_NO_PURGE.to_string(),
                        LABEL_NO_PURGE_VAL.to_string(),
                    )])
                    .build(),
            )
            .await?;
        let managed = self
            .created
            .iter()
            .map(|f| f.name.to_owned())
            .collect::<Vec<_>>();
        for volume in volume {
            let name = volume.name;
            if !managed.contains(&name) {
                api.get(&name).delete().await?;
                info!(name, "purged volume");
            }
        }
        Ok(())
    }

    pub fn merge(self, new: &mut Self) {
        new.created.extend(self.created);
        new.removed.extend(self.removed);
    }
}

#[derive(Debug, PartialEq, Eq, Serialize, Deserialize, Clone, Default)]
pub struct VolumeCreated {
    pub name: String,
    pub driver: String,
    #[serde(default)]
    pub options: HashMap<String, String>,
    #[serde(default)]
    pub labels: HashMap<String, String>,
}

impl From<VolumeCreated> for VolumeCreateOptsBuilder {
    fn from(val: VolumeCreated) -> Self {
        VolumeCreateOpts::builder()
            .name(val.name)
            .driver(val.driver)
            .options(val.options)
            .labels(val.labels)
    }
}

impl cmp::PartialEq<VolumeInspect> for VolumeCreated {
    fn eq(&self, other: &VolumeInspect) -> bool {
        self.name == other.name
            && self.driver == other.driver
            && self.options == other.options
            && self.labels == other.labels
    }
}

direct_into_build!(VolumeCreated, VolumeCreateOptsBuilder => VolumeCreateOpts);

#[derive(Debug, PartialEq, Eq, Serialize, Deserialize, Clone, Default)]
pub struct VolumeRemoved {
    pub name: String,
    #[serde(default)]
    pub force: Option<bool>,
}
