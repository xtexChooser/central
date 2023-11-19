#!/bin/sh

/usr/sbin/crond -s /dist/cron.d -f -L /dev/stdout
