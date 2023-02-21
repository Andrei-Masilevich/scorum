# CLI Wallet Build for Scorum blockchain

Scorum blockchain was created in obsolete environment (Boost, readline, openssl, etc.) and it difficult to built it in modern linux system.
The current solution use automation containerize just to use CLI wallet feature at modern Linux system.

This container should be run locally or at the trusted server and makes in
3 steps.

(1.) Docker is required.
You can setup **docker** by auxiliary script:
```bash
    ./install.docker.sh
```
> **It require privileges escalation** because it does package installations. But the following scripts don't require privileges escalation

2. To build
```bash
    ./wallet.build.sh
```

3. To run (it builds automatically if it was not built before)
```bash
    ./wallet.sh [json]
```
* where **json** - It's the path to your secret configuration file where you encrypted private keys (and other wallet configuration) will be saved
