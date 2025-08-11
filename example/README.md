# Example

Simple Hello World Tether example.

Server:
```sh
server.exe --pubkey public-key.bin --privkey private-key.bin --tether HelloWorld.tether
Public Key: 8120e7f37b0379b92a0b737d9199beb95e3b38151f6ccbd58583b775cce44906
[2025-08-10 23:35:12.936] [info] Starting a Tether server on: localhost:1234
```

Client:

```sh
# run this in a console
HelloWorld.tether.exe
Hello World!
```