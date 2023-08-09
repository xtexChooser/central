#!/bin/bash
echo Merging android/kernel/configs
git fetch https://android.googlesource.com/kernel/configs.git main
#git merge -s ours --no-commit --allow-unrelated-histories FETCH_HEAD
#git read-tree --prefix=taotie/android-configs -u FETCH_HEAD
git merge -X subtree=taotie/android-configs FETCH_HEAD
git commit -s -S -m "Merge android/kernel/configs from upstream"
