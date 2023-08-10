#!/bin/bash
# shellcheck disable=SC2034,SC1091,SC2068

if [[ "$(pwd)" =~ .*\/taotie\/.* ]]; then
    echo Please run in the kernel source tree, but not taotie subdir.
    exit
fi
if [[ "$1" == "" ]]; then
    echo "Usage: ./build.sh <device> <action>"
    exit
fi

ROOT_DIR=.
KERNEL_DIR=.
KDIR=$(pwd)
OUT_DIR=output
OUT_DIR_F=$(readlink -e "$OUT_DIR")
export KDIR OUT_DIR OUT_DIR_F
mkdir -p "$OUT_DIR" "$OUT_DIR_F/kernel"

if [[ "$1" == "all" ]]; then
    for device in taotie/devices/*
    do
        echo Running for "$device"
        ./build.sh "$device" ${@:2}
    done 
elif [[ "$1" == "n" ]]; then
    exec "taotie/scripts/$2.sh" ${@:3}
fi

# Load device config
DDIR=taotie/devices/$1
if [[ "$DDIR" =~ taotie\/devices\/taotie\/devices\/.* ]]; then
    DDIR=$1
fi
if [[ ! -e "$DDIR" ]]; then
    exec echo "build config $1 not found"
fi
export DDIR

. "$DDIR/deviceinfo"
export ARCH

CLANG_PREBUILT_BIN=../clang/clang-r498229/bin/
export PATH=$CLANG_PREBUILT_BIN:$PATH

# Build kernel make args
KMAKE_ARGS="\
KCONFIG_CONFIG=$(readlink -e "$DDIR")/defconfig.merge \
ARCH=$ARCH \
CC=clang \
LD=ld.lld \
LLVM=$LLVM \
CROSS_COMPILE=$CROSS_COMPILE \
CROSS_COMPILE_ARM32=$CROSS_COMPILE_ARM32 \
DEPMOD=$DEPMOD \
O=$OUT_DIR/build \
INSTALL_PATH=$OUT_DIR_F/kernel \
INSTALL_MOD_PATH=$OUT_DIR_F/modules \
INSTALL_MOD_STRIP=1 \
INSTALL_DTBS_PATH=$OUT_DIR_F/dtbs \
"
export KMAKE_ARGS

echo "$KMAKE_ARGS"

if [[ -e "taotie/scripts/$2.sh" ]]; then
    exec "taotie/scripts/$2.sh" ${@:3}
elif [[ -e "$DDIR/scripts/$2.sh" ]]; then
    exec "$DDIR/scripts/$2.sh" ${@:3}
fi

if [[ "$2" == "build" ]]; then
    set -xe
    # shellcheck disable=SC2086
    {
        make -j64 $KMAKE_ARGS $MAKE_GOALS
        kernelrelease=$(make $KMAKE_ARGS kernelrelease)
        rm -rf $OUT_DIR_F/kernel/*
        find "$OUT_DIR_F/modules/lib/modules" -mindepth 1 -maxdepth 1 -not -name "$kernelrelease" -print0 | xargs -0 rm -rf --
        make -j8 $KMAKE_ARGS $INSTALL_GOALS
        rm -f "$OUT_DIR"/modules/lib/modules/*/build "$OUT_DIR"/modules/lib/modules/*/source
    }
elif [[ "$2" == "mrproper" || "$2" == "clean" ]]; then
    exec make -j4 ARCH="$ARCH" "$2"
elif [[ "$2" == "make" ]]; then
    # shellcheck disable=SC2068,SC2086
    exec make -j64 $KMAKE_ARGS ${@:3}
fi
 