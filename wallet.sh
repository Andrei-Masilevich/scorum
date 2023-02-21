#!/bin/bash

THIS_DIR=$(cd $(dirname ${BASH_SOURCE}) && pwd)
source ${THIS_DIR}/wallet.contrib/lib_wallet.sh

function main()
{
    if [ -z $1 ]; then
        print_error "Wallet configuration file is required!"
        exit 1
    fi

    init_docker

    local CREATE_NEW=1
    if [ -f $1 ]; then
        if [[ -n "$(fgrep "\"ws_server\":" $1)" && -n "$(fgrep "\"chain_id\":" $1)" ]]; then
            CREATE_NEW=
        fi
    fi

    local WALLET_CONFIG=$1
    if ((CREATE_NEW)); then
        cat <<% >${WALLET_CONFIG}
{
    "ws_server": "wss://prodnet.scorum.com",
    "chain_id": "db4007d45f04c1403a7e66a5c66b5b1cdfc2dde8b5335d1d2f116d592ca3dbb1"
}
%
    fi

    local INSTALLED_SERVICE=$($DOCKER images -q ${SCORUM_WALLET_IMAGE})
    if [ ! -n "${INSTALLED_SERVICE}" ]; then
        . ${THIS_DIR}/wallet.build.sh
    fi

    exec $DOCKER run --rm --net=host \
        -it -v ${WALLET_CONFIG}:/home/appuser/app/wallet.json \
        --entrypoint './cli_wallet' ${SCORUM_WALLET_IMAGE}
}

main $@
