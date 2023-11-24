#!/usr/bin/env bash

PATH=/dist/bin:/dist/sbin:$PATH

mkdir -p /srv/exopages/xtex-wiki-bot
rm -f pub config
ln -s /srv/exopages/xtex-wiki-bot pub
ln -s /srv/run/config config
exec dinit -d /dist/dinit.d --container -t xt-bot
