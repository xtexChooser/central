#!/usr/bin/env sh
set -e

command -v curl >/dev/null 2>&1 || apk add curl

[[ "${CI_WORKFLOW_NAME:-}" == "" ]] && echo "CI_WORKFLOW_NAME is not available" && exit 1
[[ "${CI_PIPELINE_NUMBER:-}" == "" ]] && echo "CI_PIPELINE_NUMBER is not available" && exit 1
[[ "${CI_PIPELINE_EVENT:-}" == "" ]] && echo "CI_PIPELINE_EVENT is not available" && exit 1
[[ "${CI_STEP_URL:-}" == "" ]] && echo "CI_STEP_URL is not available" && exit 1
[[ "${NTFY_TOKEN:-}" == "" ]] && echo "NTFY_TOKEN is not available" && exit 1

pipeline=${CI_WORKFLOW_NAME#build-}
if [[ "$VERSION" != "" ]]; then
	pipeline="$pipeline $VERSION"
fi

title="$pipeline build ended"
desc="CI-Pipeline-Number: $CI_PIPELINE_NUMBER
CI-Link: <$CI_STEP_URL>
Triggerred-by-event: $CI_PIPELINE_EVENT"

curl \
	-H "Authorization: Bearer $NTFY_TOKEN" \
	-H "X-Title: $title" \
	-H "X-Actions: view, View on CI, $CI_STEP_URL" \
	-H "X-Tags: pipline-$pipeline" \
	-d "$desc" \
	-SL \
	https://ntfy.xvnet.eu.org/publogs
