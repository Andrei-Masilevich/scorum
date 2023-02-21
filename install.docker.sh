#!/bin/bash

THIS_DIR=$(cd $(dirname ${BASH_SOURCE}) && pwd)
source ${THIS_DIR}/wallet.contrib/lib_wallet.sh

function main()
{
    init_sudo

    if [ -n "$(which docker)" ]; then
        local USER_GROUPS_=($(${UNSUDO} groups))
        local SUDO_REQUIRED=1
        for group in "${USER_GROUPS_[@]}"; do 
            if [ "${group}" == "docker" ]; then
                SUDO_REQUIRED=0
                break
            fi
        done

        if (( SUDO_REQUIRED )); then
            print_error "Docker launch requires privileges escalation. Install proparly group or use SUDO."
            return 0
        fi    
    else
        if [ -n "${SUDO}" ]; then
            print_error "Docker installation requires privileges escalation."
            answer_yes "Continue with SUDO?" USE_SUDO    
            if (( USE_SUDO != 1 )); then
                return 1
            fi
        fi
        ${SUDO} curl -sSL get.docker.com | sh
        if [[ $? -ne 0 || -z $(which docker) ]]; then
            print_error "Docker can not been install automatically!"
            exit 1
        fi
    fi
}

main


