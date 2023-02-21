FROM phusion/baseimage:0.9.19 as builder

RUN \
    apt-get update && \
    apt-get install -y \
    autoconf \
    automake \
    autotools-dev \
    bsdmainutils \
    build-essential \
    cmake \
    doxygen \
    git \
    libboost-all-dev \
    libicu-dev \
    libreadline-dev \
    libssl-dev \
    libtool \
    ncurses-dev \
    pbzip2 \
    pkg-config \
    python3 \
    python3-dev \
    nginx \
    fcgiwrap \
    s3cmd \
    awscli \
    jq \
    wget \
    gdb \
    && \
    apt-get install -y libicu55 libreadline6 && \
    apt-get install -y curl apt-transport-https ca-certificates && \
    apt-get --only-upgrade install -y libgnutls-openssl27 libgnutls30 ubuntu-advantage-tools && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    wget https://bootstrap.pypa.io/pip/3.5/get-pip.py && \
    python3 get-pip.py && \
    pip3 install gcovr && \
    pip3 install jinja2 && \
    mkdir -p /usr/local/src/scorum

WORKDIR /usr/local/src/scorum
COPY ./CMakeLists.txt .
COPY ./CMakeModules ./CMakeModules
COPY ./genesis ./genesis
COPY ./programs ./programs
COPY ./libraries ./libraries
COPY ./external_plugins ./external_plugins
COPY ./tests ./tests
COPY ./.git ./.git

RUN \
    cd /usr/local/src/scorum && \
    mkdir build && \
    cd build && \
    cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local/scorumd-full \
    -DSCORUM_LIVE_TESTNET=${LIVE_TESTNET} \
    -DSCORUM_LOW_MEMORY_NODE=OFF \
    -DSCORUM_CLEAR_VOTES=OFF \
    -DSCORUM_SKIP_BY_TX_ID=OFF \
    .. && \
    find .. -type f -name '*.py' -exec chmod +x {} \; && \
    find .. -type f -name '*.sh' -exec chmod +x {} \; && \
    make -j$(nproc) && \
    ./tests/wallet_tests/wallet_tests && \
    make install && \
    cd / && \
    rm -rf /usr/local/src/scorum

# -----------------------------------
# Install application sources and run
FROM phusion/baseimage:0.9.19 as runtime

RUN \
    apt-get update && \
    apt-get install -y libicu55 libreadline6 && \
    apt-get install -y curl apt-transport-https ca-certificates && \
    apt-get --only-upgrade install -y libgnutls-openssl27 libgnutls30 ubuntu-advantage-tools && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN useradd -m -d /home/appuser/ appuser && \
    mkdir /home/appuser/app && \
    chown -R appuser /home/appuser/app

USER appuser
WORKDIR /home/appuser/app

COPY --from=builder --chown=appuser:appuser \
    /usr/local/scorumd-full/bin/cli_wallet \
    ./cli_wallet

# This command will be overwritten from launch script
CMD ["./cli_wallet", "-v"]
