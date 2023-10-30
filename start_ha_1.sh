#!/bin/bash

DATABASE_URL="postgresql://rauthy:123SuperSafe@localhost:5432/rauthy" \
HA_MODE=true \
HA_HOSTS="http://127.0.0.1:8001,http://127.0.0.1:8002,http://127.0.0.1:8003" \
HOSTNAME_OVERWRITE="127.0.0.1:8001" \
cargo run $1