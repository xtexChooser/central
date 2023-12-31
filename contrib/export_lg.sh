#!/usr/bin/env bash

# Maintainer: xtex <xtexchooser@duck.com>

# https://git.exozy.me/xtex/oi-learn/raw/branch/main/contrib/export_lg.sh

# This is free and unencumbered software released into the public domain.
# For more information, please refer to <http://unlicense.org/>

# export-lg.sh: Export all solution codes from Luogu
# Usage: VAR1=VALUE VAR2=VAL bash <(curl -sSL https://git.exozy.me/xtex/oi-learn/raw/branch/main/contrib/export_lg.sh)
#   Or put variables in .env file
# Variables:
#   General:
#     OUT: output directory
#     CURL_ARGS: custom curl arguments, separated by spaces
#     LG_SUBMIT_STATUS: submit status to filter for, default is "AC", set to empty to disable filtering
#     FORMATTER_(source file extension): formatter
#     USE_CLANG_FORMAT: use clang-format for c and c++
#     DEBUG_CMD: debug all shell commands
#     DEBUG_API: debug api requests
#   Cookies:
#     COOKIES: cookies, in form of "__client_id=xxx _uid=xxx somekey=value"
#     FIREFOX_COOKIES: get login cookies from firefox
#     FIREFOX_INST_DIR: Firefox installations directory
#     ONLY_RUNNING_FF: only load cookies from running Firefox instances
#     IGNORE_MISSING_COOKIES: ignore missing cookies for authentication

METADATA_VERSION=1

set -e

die() {
	echo -e "$1" >/dev/stderr
	exit 1
}

: "${OUT:=luogu/}"
: "${CURL_ARGS:=}"
: "${LG_SUBMIT_STATUS:=AC}"
: "${USE_CLANG_FORMAT:=false}"
: "${DEBUG_API:=false}"
: "${DEBUG_CMD:=false}"
: "${COOKIES:=}"
: "${FIREFOX_COOKIES:=false}"
: "${FIREFOX_INST_DIR:=$HOME/.mozilla/firefox/}"
: "${ONLY_RUNNING_FF:=false}"
: "${IGNORE_MISSING_COOKIES:=false}"

# shellcheck source=/dev/null
! [[ -f ".env" ]] || source .env

"$DEBUG_CMD" && set -x
# shellcheck disable=SC2034
"$USE_CLANG_FORMAT" && {
	FORMATTER_c="clang-format -i"
	FORMATTER_cc="clang-format -i"
}

mkdir -p "$OUT"

declare -a curlArgs
[[ "$CURL_ARGS" != "" ]] && readarray curlArgs <<<"$CURL_ARGS"
curlArgs+=('-sSL' '--retry' '3')
curlArgs+=('-H' 'x-luogu-type: content-only' '-H' 'referer: https://www.luogu.com.cn/')

declare -A cookies
extractFirefoxCookies() {
	[[ -d "$FIREFOX_INST_DIR" ]] || die "Firefox is not found at $FIREFOX_INST_DIR"
	checkCommand sqlite3
	for db in "$FIREFOX_INST_DIR"/*/cookies.sqlite; do
		# cookies databases whose firefox is running are locked
		local is_db_locked=false
		sqlite3 --readonly "$db" 'SELECT NULL FROM moz_cookies;' &>/dev/null || is_db_locked=true
		"$ONLY_RUNNING_FF" && ! "$is_db_locked" && continue

		local dbfile="$db"
		if "$is_db_locked"; then
			dbfile="$(mktemp)"
			cat <"$db" >"$dbfile"
		fi

		readarray -d$'\n' cklines <<<"$(sqlite3 --tabs --noheader --readonly "$dbfile" \
			"SELECT name, value FROM moz_cookies WHERE host == '.luogu.com.cn';")"
		for ckline in "${cklines[@]}"; do
			local key value
			key=$(cut -f1 <<<"$ckline") value=$(cut -f2 <<<"$ckline")
			{ [[ "$key" == '' ]] || [[ "$value" == '' ]]; } && continue
			cookies["$key"]="$value"
		done

		"$is_db_locked" && rm "$dbfile"
	done
}

checkCookie() {
	"$IGNORE_MISSING_COOKIES" && return
	[[ "${cookies["$1"]:-}" != "" ]] || die "Missing cookie: $1\nHave you logged into Luogu in your browser?"
}

initCookies() {
	! "$FIREFOX_COOKIES" || extractFirefoxCookies
	[[ "$COOKIES" == "" ]] || {
		local -a cks
		readarray -d ' ' -t cks <<<"$COOKIES"
		for ck in "${cks[@]}"; do
			local key value
			key=$(cut -d'=' -f1 <<<"$ck") value=$(cut -d'=' -f2 <<<"$ck")
			cookies["$key"]="$value"
		done
	}

	[[ ${#cookies[@]} -gt 0 ]] || die "No cookies are loaded"
	echo "Loaded cookies: ${!cookies[*]}"
	checkCookie "__client_id"
	checkCookie "_uid"

	for ck in "${!cookies[@]}"; do
		curlArgs+=("--cookie" "$ck=${cookies["$ck"]}")
	done
}

checkCommand() {
	command -v "$1" >>/dev/null || die "Command $1 is not found"
}

callLgApi() {
	checkCommand curl
	local url cmd args=()
	url="https://www.luogu.com.cn$1"
	shift
	while [[ $# -gt 0 ]]; do
		[[ "$(cut -d'=' -f2 <<<"$1")" != "" ]] || {
			shift
			continue
		}
		args+=('--url-query' "$1")
		shift
	done
	cmd=("curl" "${curlArgs[@]}" "${args[@]}" "$url")

	if "$DEBUG_API"; then
		echo "call api:" "${cmd[@]}" >/dev/stderr
		"${cmd[@]}" | tee /dev/stderr
		printf '\n' >/dev/stderr
	else
		"${cmd[@]}"
	fi
}

getLgApi() {
	local endpoint
	endpoint="$(jq -r ".routes[\"$1\"]" <<<"$lgConfig")"
	[[ "$endpoint" != "" ]] || die "Luogu API route $1 not found in config"
	echo "$endpoint"
}

fetchPage() {
	echo Fetching page $page
	local -a lines
	readarray -t lines < <(callLgApi "$(getLgApi "record.list")" page="$page" user="$lgUid" status="$lgTargetStatus" | jq -r '.currentData.records.result | map(.problem.pid + "|" + (.id | tostring)) | reverse | .[]')
	for ln in "${lines[@]}"; do
		local pid rid
		pid="$(cut -d'|' -f1 <<<"$ln")"
		rid="$(cut -d'|' -f2 <<<"$ln")"
		records["$pid"]="$rid"
	done
}

fetchRecord() {
	{ [[ -f "$OUT/$1.txt" ]] &&
		grep -E "^Metadata-Version: $METADATA_VERSION$" "$OUT/$1.txt" &>/dev/null &&
		grep -E "^Record-Id: ${records["$1"]}$" "$OUT/$1.txt" &>/dev/null; } && return
	echo Fetching "$1"

	local record pid srcext srcfile metafile runfmt
	record="$(callLgApi "$(getLgApi "record.show" | sed -e "s/{id}/${records["$1"]}/")" | jq ".currentData.record")"
	pid="$(jq -r ".problem.pid" <<<"$record")"

	srcext="$(jq -r ".codeLanguages.[] | select(.value == $(jq -r ".language" <<<"$record")) | .fileExtensions[0]" <<<"$lgConfig")"
	if [[ "$srcext" == "null" ]] || [[ "$srcext" == "txt" ]]; then
		srcfile="$OUT/${pid}_source.txt"
		runfmt=false
	else
		srcfile="$OUT/$pid.$srcext"
		runfmt=true
	fi
	metafile="$OUT/$pid.txt"

	rm -f "$OUT/$pid".*
	jq -r ".sourceCode" <<<"$record" >"$srcfile"
	# shellcheck disable=SC2153
	if "$runfmt"; then
		local fmt
		fmt=$(eval echo "\${FORMATTER_${srcext}:-}")
		# shellcheck disable=SC2086
		[[ "${fmt}" == "" ]] || eval ${fmt} "$srcfile"
	fi

	cat >"$metafile.tmp" <<EOF
Metadata-Version: $METADATA_VERSION
Problem-Id: $pid
Record-Id: ${records["$1"]}
Timestamp: $(jq -r ".submitTime" <<<"$record")
Status: $(jq -r ".recordStatus.[] | select(.id == $(jq -r ".status" <<<"$record")) | .name" <<<"$lgConfig")
Score: $(jq -r ".score" <<<"$record")/$(jq -r ".problem.fullScore" <<<"$record")
Language: $(jq -r ".codeLanguages.[] | select(.value == $(jq -r ".language" <<<"$record")) | .name" <<<"$lgConfig")

Test-Result:
EOF

	local subtasks
	subtasks="$(jq -r ".detail.judgeResult.subtasks | length" <<<"$record")"
	for ((subtask = 0; subtask < subtasks; subtask++)); do
		local st stid
		st="$(jq ".detail.judgeResult.subtasks | to_entries | map(.value)[$subtask]" <<<"$record")"
		stid="$(jq -r ".id" <<<"$st")"

		printf "\tSubtask: %s\n" "$stid" >>"$metafile.tmp"
		local -a cases
		readarray -t cases < <(jq -c -r ".testCases | to_entries | map(.value) | sort_by(.id).[]" <<<"$st")
		for case in "${cases[@]}"; do
			printf "\t\tCase: %s %s %s %s %s\n" \
				"$(jq -r ".id" <<<"$case")" \
				"$(jq -r ".recordStatus.[] | select(.id == $(jq -r ".status" <<<"$case")) | .shortName" <<<"$lgConfig")" \
				"$(jq -r "if .time < 1000 then (.time | tostring) + \" ms\" else (.time / 1000 | tostring) + \" s\" end" <<<"$case")" \
				"$(jq -r "if .memory < 1024 then (.memory | tostring) + \" KiB\" else ((.memory / 1024 * 100 | ceil) / 100 | tostring) + \" MiB\" end" <<<"$case")" \
				"$(jq -r ".description" <<<"$case")" \
				>>"$metafile.tmp"
		done
	done

	mv "$metafile.tmp" "$metafile"
}

{
	initCookies
	checkCommand jq

	{
		lgConfig="$(callLgApi '/_lfe/config')"
		if [[ "$LG_SUBMIT_STATUS" != "" ]]; then
			lgTargetStatus="$(jq ".recordStatus.[] | select(.shortName == \"$LG_SUBMIT_STATUS\" or .name == \"$LG_SUBMIT_STATUS\").id" <<<"$lgConfig")"
			[[ "$lgTargetStatus" == "" ]] && die "Filter status not found: $LG_SUBMIT_STATUS\nAvailable filters: $(jq -r ".recordStatus.[].shortName" <<<"$lgConfig" | sort | uniq | tr '\n' ' ')"
			echo "Luogu target status id: $lgTargetStatus"
		else
			lgTargetStatus=""
		fi
	}

	lgUid="${cookies["_uid"]}"
	lgUser="$(callLgApi "$(getLgApi "api.user.get_info" | sed -e "s/{uid}/$lgUid/")" | jq -r ".user.name")"
	[[ "$lgUser" != "" ]] || die "Failed to get user name"
	echo "Luogu username: $lgUser"

	pages="$(callLgApi "$(getLgApi "record.list")" user="$lgUid" status="$lgTargetStatus" | jq ".currentData.records | (.count / .perPage) | ceil")"

	declare -A records

	for ((page = pages; page > 0; page--)); do fetchPage $page; done
	echo "${#records[@]}" records found

	for pid in "${!records[@]}"; do fetchRecord "$pid"; done
	echo Completed
}
