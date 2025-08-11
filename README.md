## Tether

Tether is a project designed to extract groups of CPU instructions from a compiled binary (x86_64 PE file) and execute them exclusively on a remote server. The concept of offloading sensitive or computationally intensive code to a remote system is not new. Various approaches have been explored over the years, some of which share conceptual similarities with Tether:

- Remote Procedure Calls (RPC)
    - RPC is a long-established technique that allows a program to execute code on another machine as if it were local. In this model, function calls made on the client are serialized, sent over the network, executed on the server, and the results returned back.
- Remote Graphics Processing
    - Remote graphics systems such as early thin clients, X11 forwarding, or modern GPU virtualization which sends rendering commands or framebuffers over the network so that the client can display output without owning the full graphics stack.

### Origins
---
Tether originated as an internal side project at Back Engineering Labs, aiming to explore the possibilities of securing software by relocating sensitive computational routines to a remote server after the software was already compiled. This post-compilation approach means that protection can be added without modifying source code or development workflows.

The name "Tether" was decided upon as it reminded us of old [IPhone jailbreak exploits that required you to re-run them after a reboot](https://www.idownloadblog.com/2019/11/21/types-of-jailbreaks/)

### Open Source Details
---
For clarity, the Tether server code is completely free and open source, you must host your own Tether servers. [CodeDefender](https://codedefender.io/) is used to extract instructions from the program and statically link the embedded Tether client. The embedded client code is statically linked into a program post-compilation by [CodeDefender](https://codedefender.io/). You are free to fork and manipulate server code as you see fit, however the client code is more closely integrated into our CodeDefender SaaS platform. If you wish to alter the embedded client code, please contact us at `contact@back.engineering`.

### Networking Protocol

- Tether uses ENet, a lightweight UDP networking library. The setup and teardown of UDP connections is much faster than that of a TCP connection. ENet offers reliable packet delivery, including proper ordering. It also offers multiple channels per-connection.

- Tether uses [monocypher](https://monocypher.org/), a very tiny, but powerful crypto library. When a connection is established, there is an exchange of `x25519` public keys. A shared secret is then derived using these key's which is then further hashed via `blake2b` to derive a session key. This session key is then used with `XChaCha20` to encrypt CPU context states. In addition, the client is statically linked with the expected remote servers public key to prevent any man-in-the-middle attacks. This cryptographic handshake is relatively quick to setup and use.

## Usage - Key Generation

The Tether server requires a few files to operate. Firstly, you need a `.tether` file, which contains extracted instructions from a program. You can create these files by signing up to [CodeDefender](https://codedefender.io/) and using the SaaS.

Secondly, you will need to generate a public/private key pair using our `keygen` tool. This will produce a public/private key in a folder. The public key is required by the SaaS to pin it. An example of public private keys would be:

```sh
# Execute keygen.exe
Secret key: 28923ae94d2ae8f4dcad8cdc05951e7355f3e6f61322b1bcb52c743e7f9674aa
Public key: d308825a6eb92d6906ec00c13978b7feaf8cf02c42ff4bc7fc9c535ae5321438
```

## Usage - Tether Server

Once you have your `.tether` file, public key, and private key, you can run a Tether server.

```sh
server.exe -h
Error: --pubkey, --privkey, and --tether are required.

Tether Server
Usage:
  server [OPTIONS]

Options:
  -h, --host HOST        Host address to bind (default: 0.0.0.0)
  -p, --port PORT        Port number to bind (default: 1234)
  -c, --max-clients      Max number of connections at once (default 128)
      --pubkey FILE      Path to public key file (required)
      --privkey FILE     Path to private key file (required)
      --tether FILE      Path to tether file (required)
      --help             Show this help message
```

## Build Instructions

Currently this codebase will only compile and run on Windows as there are a handful of API's that are Windows specific. If there is interest in Linux support please open a PR.

Requires:

- [Visual Studios 2022](https://visualstudio.microsoft.com/)
- [LLVM-MSVC v143](https://github.com/backengineering/llvm-msvc/releases/download/llvm-msvc-v777.1.9/llvm-msvc_X86_64_installer.exe)
- [CMake](https://cmake.org/download/)

```
cmake -B .build -T LLVM-MSVC_v143 -A x64
cd .build
cmake --build . --config Release
```