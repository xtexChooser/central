#!/bin/bash
make -C "$DDIR" defconfig.base
# shellcheck disable=SC2086
cd "$KDIR" && eval $KMAKE_ARGS KCONFIG_CONFIG="$DDIR/defconfig.merge" \
    scripts/kconfig/merge_config.sh \
    "$DDIR/defconfig.base" \
    "taotie/android-configs/u/android-6.1/android-base.config" \
    "$DDIR/defconfig.ext"
rm -f "$DDIR/defconfig.merge.old"
