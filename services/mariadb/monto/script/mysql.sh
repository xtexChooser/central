#!/usr/bin/env bash
set -e
exec podman exec -it monto mysql -u root "$@"
