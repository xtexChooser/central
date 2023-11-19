#!/usr/bin/env bash

PATH=/dist/bin:/dist/sbin:$PATH

exec dinit -d /dist/dinit.d --container -t xt-bot
