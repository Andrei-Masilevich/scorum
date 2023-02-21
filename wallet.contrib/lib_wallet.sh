SCORUM_WALLET_IMAGE=scorum.wallet

function print_error()
{
    echo $@ >&2
}

function init_sudo()
{
    if [[ $EUID -ne 0 ]]; then
        SUDO="sudo"
        LOGIN=${USER}
        UNSUDO=""
    else
        SUDO=""
        LOGIN=$(who | awk '{print $1; exit}')
        UNSUDO="sudo -u ${LOGIN}"
    fi
}

function __answer()
{
    local ANSW=$1
    local DEFAULT_YES=$2
    declare -n _YES_RESULT=$3
    local SELECTION_PROMPT=$4

    if [[ ! -n "${SELECTION_PROMPT}" ]]; then
        SELECTION_PROMPT='[Y/n]';
        if (( $DEFAULT_YES < 1 )); then
            SELECTION_PROMPT='[N/y]'
        fi
    fi

    local ANSW_="${ANSW} ${SELECTION_PROMPT}: "
    read -p "${ANSW_}" ANSW_RESULT_
    until [[ -z "$ANSW_RESULT_" || "$ANSW_RESULT_" =~ ^([YyNn]+|[Yy]+[Ee]+[Ss]+|[Nn]+[Oo]+)$ ]]; do
        read -p "${ANSW_}" ANSW_RESULT_
    done

    if [ -z ${ANSW_RESULT_} ]; then
        if (( $DEFAULT_YES > 0 )); then
            _YES_RESULT=1
        fi
    elif [[ ! "$ANSW_RESULT_" =~ ^([Nn]+|[Nn]+[Oo]+)$ ]]; then
        _YES_RESULT=1
    fi
    ANSW_RESULT_=
}

function answer_yes()
{
    local ANSW=$1
    declare -n YES_RESULT=$2

    __answer "${1}" 1 YES_RESULT_ "$3"
    YES_RESULT=$YES_RESULT_
    YES_RESULT_=
}

function answer_no()
{
    local ANSW=$1
    declare -n YES_RESULT=$2

    __answer "${1}" 0 YES_RESULT_ "$3"
    YES_RESULT=$YES_RESULT_
    YES_RESULT_=
}


function init_docker()
{
    init_sudo

    if [ -z $(which docker) ]; then
        print_error "Docker is required!"
        exit 1
    fi

    if [ -n "${SUDO}" ]; then
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
            answer_no "Continue with SUDO?" USE_SUDO
            if (( USE_SUDO )); then
                DOCKER="$SUDO docker"
                $DOCKER images -q ${SCORUM_WALLET_IMAGE}
            else
                exit 1
            fi
            return 0
        fi
    fi

    DOCKER="docker"
}
