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
export KDIR
export OUT_DIR
if [[ ! -e $OUT_DIR ]]; then
    mkdir $OUT_DIR
fi

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
KMAKE_ARGS="O=$OUT_DIR \
ARCH=$ARCH \
CC=clang \
LD=ld.lld \
LLVM=$LLVM \
CROSS_COMPILE=$CROSS_COMPILE \
CROSS_COMPILE_ARM32=$CROSS_COMPILE_ARM32 \
DEPMOD=$DEPMOD \
KCONFIG_CONFIG=$DDIR/defconfig.merge
"
export KMAKE_ARGS

echo "$KMAKE_ARGS"

if [[ -e "taotie/scripts/$2.sh" ]]; then
    exec "taotie/scripts/$2.sh" ${@:3}
elif [[ -e "$DDIR/scripts/$2.sh" ]]; then
    exec "$DDIR/scripts/$2.sh" ${@:3}
fi

if [[ "$2" == "build" ]]; then
    # shellcheck disable=SC2086
    exec make "-j$(nproc)" $KMAKE_ARGS $MAKE_GOALS
elif [[ "$2" == "mrproper" || "$2" == "clean" ]]; then
    exec make -j4 ARCH="$ARCH" "$2"
elif [[ "$2" == "make" ]]; then
    # shellcheck disable=SC2068,SC2086
    exec make "-j$(nproc)" $KMAKE_ARGS ${@:3}
fi
 