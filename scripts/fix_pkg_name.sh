#!/usr/bin/env bash
#
# This fixes package name="" in coverage.xml or another coverage filename
# as the only optional argument: ./fix_pkg_name.sh other-name.xml
# We default to grepping pkg name from (python) setup.cfg
# otherwise you should set the REAL_NAME environment override, eg:
#
#    REAL_NAME="re2" ./fix_pkg_name.sh
#
# or export it first in your shell env.

set -euo pipefail

failures=0
trap 'failures=$((failures+1))' ERR

COV_FILE=${1:-coverage.xml}
REAL_NAME=${REAL_NAME:-""}
VERBOSE="false"  # set to "true" for extra output

NAME_CHECK=$(grep -o 'name=""' "${COV_FILE}" || true)

# extra fix for autotools ??
sed -i -e "s|src..libs|src|" $COV_FILE

[[ -z "$NAME_CHECK" ]] && echo "No name to fix ..." && exit 0
[[ -n $REAL_NAME ]] || REAL_NAME=$(grep ^name setup.cfg | cut -d" " -f3)
[[ -n $REAL_NAME ]] && sed -i -e "s|name=\"\"|name=\"${REAL_NAME}\"|" $COV_FILE
[[ -n $REAL_NAME ]] && echo "Replaced \"\" with ${REAL_NAME} in ${COV_FILE} ..."

if ((failures != 0)); then
    echo "Something went wrong !!!"
    exit 1
fi
