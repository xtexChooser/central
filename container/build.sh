#!/usr/bin/env bash

set -xe

# Install rust
apk add -U curl musl-dev openssl-dev openssl-libs-static
#apk add curl musl-dev gcc openssl-dev openssl-libs-static
#curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal --default-toolchain nightly
# shellcheck source=/dev/null
#source "$HOME/.cargo/env"
#RUSTFLAGS='-C target-feature=+crt-static'
#export RUSTFLAGS

# Install bot
#cargo test

mkdir -p /dist
cargo install --root /dist --all-features --bins --path /build
rm -f /dist/bin/.crates.toml /dist/bin/.crates2.json

# Install dinit
apk add tar clang llvm make m4
curl --proto '=https' --tlsv1.2 -sSL -o dinit.tar.gz https://github.com/davmac314/dinit/archive/refs/heads/master.tar.gz
tar -xf dinit.tar.gz
cd dinit-master

dinitMake=(CXX=clang++ CXX_FOR_BUILD=clang++ 'CXXFLAGS_EXTRA="-O3 --static"' -j"$(nproc)")
eval "make ${dinitMake[*]}"
#eval "make ${dinitMake[*]} check check-igr"
eval "make ${dinitMake[*]} DESTDIR=/dist install"
rm -rf /dist/usr/share/man
