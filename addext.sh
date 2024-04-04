#!/usr/bin/env bash

set -e

if (($# < 2 || $# > 4)); then
	cat <<<"""Usage: $0 <ext/skin> <name> [git url] [branch]""" >&2
	exit 1
fi

kind="$1"
name="$2"
giturl="$3"
branch="$4"

case $kind in
ext | extension | extensions)
	kind=extensions
	;;
skin | skins)
	kind=skins
	;;
*)
	echo "Error: Unknown kind: $kind" >&2
	exit 1
	;;
esac
subtreePrefix=mw/$kind/"$name"

if [[ -e "$subtreePrefix" ]]; then
	echo "Error: Subtree $subtreePrefix already exists" >&2
	exit 1
fi

checkGitUrl() {
	git ls-remote -h "$1" &>/dev/null
}

tryGitUrl() {
	checkGitUrl "$1" && giturl="$1"
}

if [[ "$giturl" == "" ]]; then
	tryGitUrl https://gerrit.wikimedia.org/r/mediawiki/"$kind"/"$name" ||
		tryGitUrl https://github.com/weirdgloop/mediawiki-"$kind"-"$name" ||
		tryGitUrl https://github.com/miraheze/"$name" ||
		{
			echo "Error: Could not find extension Git repository automatically" >&2
			exit 1
		}
	echo "Found Git repository: $giturl"
fi

if [[ -z "$branch" ]]; then
	echo "Adding $giturl to $subtreePrefix as $kind $name"
	yq -i ". += [{ \"name\": \"$name\", \"kind\": \"$kind\", \"prefix\": \"$subtreePrefix\", \"git\": \"$giturl\" }]" repositories.yaml
else
	echo "Adding $giturl $branch to $subtreePrefix as $kind $name"
	yq -i ". += [{ \"name\": \"$name\", \"kind\": \"$kind\", \"prefix\": \"$subtreePrefix\", \"git\": \"$giturl\", \
		\"branch\": \"$branch\" }]" repositories.yaml
fi

yq -i '. |= sort_by(.name)' repositories.yaml

git add repositories.yaml
git commit -m "Add $kind/$name"
