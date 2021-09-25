#!/usr/bin/env bash
#
# This fixes package name="" and DTD in gcovr coverage.xml (or another
# coverage file) and takes an alternate file name as the only optional
# argument:
#  ./fix_cov_file.sh other-name.xml
# We default to grepping pkg name from (python) setup.cfg
# otherwise you should set the REAL_NAME environment override, eg:
#
#    REAL_NAME="re2" ./fix_cov_file.sh
#
# or export it first in your shell env.

set -euo pipefail

failures=0
trap 'failures=$((failures+1))' ERR

COV_FILE=${1:-coverage.xml}
REAL_NAME=${REAL_NAME:-""}
VERBOSE="false"  # set to "true" for extra output

DTD_CHECK=$(grep -o 'DOCTYPE' "${COV_FILE}" || true)
NAME_CHECK=$(grep -o 'name=""' "${COV_FILE}" || true)

[[ -z "$NAME_CHECK" && -z "$DTD_CHECK" ]] && echo "Nothing to fix ..." && exit 0
[[ -n $DTD_CHECK ]] && sed -i '/DOCTYPE/d' $COV_FILE

[[ -n $REAL_NAME ]] || REAL_NAME=$(grep ^name setup.cfg | cut -d" " -f3)
[[ -n $REAL_NAME ]] && sed -i -e "s|name=\"\"|name=\"${REAL_NAME}\"|" $COV_FILE
[[ -n $REAL_NAME ]] && echo "Replaced \"\" with ${REAL_NAME} in ${COV_FILE} ..."

if ((failures != 0)); then
    echo "Something went wrong !!!"
    exit 1
fi
