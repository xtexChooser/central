#!/bin/bash
# TWRP kernel for Samsung Galaxy S5 build script by jcadduono
# This build script is for TeamWin Recovery Project only

################### BEFORE STARTING ################
#
# download a working toolchain and extract it somewhere and configure this
# file to point to the toolchain's root directory.
# I highly recommend using a Linaro GCC 4.9.x arm-linux-gnueabihf toolchain.
# Download it here:
# https://releases.linaro.org/components/toolchain/binaries/4.9-2016.02/arm-linux-gnueabihf/
#
# once you've set up the config section how you like it, you can simply run
# ./build.sh
#
##################### VARIANTS #####################
#
# unified = kltexx, kltecan, kltetmo, klteub, kltespr, kltedv
#           G900F,  G900W8,  G900T,   G900M,  G900P,   G900I
#
# duos    = klteduos
#           G900FD
#
# chn     = kltezn, kltezm,    klteduoszn, klteduoszm, klteduosctc
#           G9006V, SM-G9008V, SM-G9006W,  SM-G9008W,  SM-G9009W
#
# kdi     = kltekdi, kltedcm
#           SCL23,   SC-04F
#
# skt     = kltektt, klteskt, kltelgt
#           G900K,   G900S,   G900L
#
###################### CONFIG ######################

# root directory of NetHunter klte git repo (default is this script's location)
RDIR=$(pwd)

[ "$VER" ] ||
# version number
VER=$(cat "$RDIR/VERSION")

# directory containing cross-compile arm toolchain
TOOLCHAIN=/mnt/src2/kltechnduo/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf

# amount of cpu threads to use in kernel make process
THREADS=16

############## SCARY NO-TOUCHY STUFF ###############

export ARCH=arm
export CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-gnueabihf-

[ "$DEVICE" ] || DEVICE=klte
[ "$TARGET" ] || TARGET=twrp
[ "$1" ] && VARIANT=$1
[ "$VARIANT" ] || VARIANT=unified
DEFCONFIG=${TARGET}_${DEVICE}_defconfig
VARIANT_DEFCONFIG=variant_${DEVICE}_${VARIANT}

ABORT()
{
	echo "Error: $*"
	exit 1
}

[ -f "$RDIR/arch/$ARCH/configs/${DEFCONFIG}" ] ||
abort "Config $DEFCONFIG not found in $ARCH configs!"

[ -f "$RDIR/arch/$ARCH/configs/$VARIANT_DEFCONFIG" ] ||
abort "Device variant/carrier $VARIANT not found in $ARCH configs!"

export LOCALVERSION="$TARGET-$DEVICE-$VARIANT-$VER"

KDIR=$RDIR/build/arch/arm/boot

CLEAN_BUILD()
{
	echo "Cleaning build..."
	cd "$RDIR"
	rm -rf build
}

SETUP_BUILD()
{
	echo "Creating kernel config for $LOCALVERSION..."
	cd "$RDIR"
	mkdir -p build
	make -C "$RDIR" O=build "$DEFCONFIG" \
		VARIANT_DEFCONFIG="$VARIANT_DEFCONFIG" \
		|| ABORT "Failed to set up build"
}

BUILD_KERNEL()
{
	echo "Starting build for $LOCALVERSION..."
	while ! make -C "$RDIR" O=build -j"$THREADS"; do
		read -p "Build failed. Retry? " do_retry
		case $do_retry in
			Y|y) continue ;;
			*) return 1 ;;
		esac
	done
}

BUILD_DTB()
{
	echo "Generating dtb.img..."
	"$RDIR/scripts/dtbTool/dtbTool" -o "$KDIR/dtb.img" "$KDIR/" -s 2048 || ABORT "Failed to generate dtb.img!"
}

CLEAN_BUILD && SETUP_BUILD && BUILD_KERNEL && BUILD_DTB && echo "Finished building $LOCALVERSION!"
