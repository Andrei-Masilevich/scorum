#!/bin/bash

THIS_DIR=$(cd $(dirname ${BASH_SOURCE}) && pwd)
source ${THIS_DIR}/wallet.contrib/lib_wallet.sh

function main()
{
    init_docker

    local R_SUBMODULES=${THIS_DIR}/libraries/fc/vendor
    if [[ ! -d ${R_SUBMODULES} || ! -n "$(ls -A ${R_SUBMODULES})" ]]; then
        print_error "Not all related libraries was loaded!"
        print_error "Perhaps you forget to execute recursive submodule synchronization:"
        print_error "git submodule update --init --recursive; git submodule sync"
        exit 1
    fi

    pushd ${THIS_DIR} > /dev/null
    $DOCKER build -f wallet.contrib/Dockerfile -t ${SCORUM_WALLET_IMAGE} .
    popd > /dev/null
}

main