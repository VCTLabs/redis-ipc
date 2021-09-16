#!/usr/bin/env bash
#
# this runs a local redis-server and accepts start|stop|status args;
# we use this in tox pre/post commands to run tests that require
# a redis server listening on the redis-ipc socket
# note: the default command is status

set -euo pipefail

failures=0
trap 'failures=$((failures+1))' ERR

CMD_ARG=${1:-status}
PORT=${PORT:-0}
VERBOSE="false"  # set to "true" for extra output
export RIPC_RUNTIME_DIR=${RIPC_RUNTIME_DIR:-/tmp}

echo "Using socket runtime dir: ${RIPC_RUNTIME_DIR}"

if [[ "${CMD_ARG}" = "status" ]]; then
    [[ "${VERBOSE}" = "true" ]]  && echo "pinging redis-server on local socket..."
    redis-cli -s ${RIPC_RUNTIME_DIR}/redis-ipc/socket ping
fi

if [[ "${CMD_ARG}" = "start" ]]; then
    [[ "${VERBOSE}" = "true" ]]  && echo "starting redis-server on local socket..."
    mkdir -p ${RIPC_RUNTIME_DIR}/redis-ipc/
    redis-server --port ${PORT} --pidfile ${RIPC_RUNTIME_DIR}/redis.pid --unixsocket ${RIPC_RUNTIME_DIR}/redis-ipc/socket --unixsocketperm 600 &
    sleep 1
    redis-cli -s ${RIPC_RUNTIME_DIR}/redis-ipc/socket config set save ""
fi

if [[ "${CMD_ARG}" = "stop" ]]; then
    [[ "${VERBOSE}" = "true" ]]  && echo "killing redis-server on local socket in 1 sec..."
    sleep 1
    cat ${RIPC_RUNTIME_DIR}/redis.pid | xargs kill
fi

if ((failures == 0)); then
    echo "Success"
else
    echo "Something went wrong"
    exit 1
fi
