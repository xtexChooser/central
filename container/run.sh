#!/usr/bin/env bash

PATH=/dist/bin:/dist/sbin:$PATH

mkdir -p /srv/exopages/xtex-wiki-bot
ln -s /srv/exopages/xtex-wiki-bot pub
ln -s /srv/run/mwbot-mcwzh.toml mwbot-mcwzh.toml
exec dinit -d /dist/dinit.d --container -t xt-bot
