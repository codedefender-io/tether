#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define ENET_IMPLEMENTATION
#include <enet.h>
#include <bit>
#include <cstdint>
#include <format>
#include <fstream>
#include <init.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <cli.hpp>
#include <server.hpp>
#include <tether.hpp>

std::vector<uint8_t> readbytes(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file) {
    return {};
  }

  std::streamsize size = file.tellg();
  if (size <= 0) {
    return {};
  }
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    return {};
  }

  return buffer;
}

int main(int argc, char** argv) {
  cli_args_t args;
  if (!parse_args(argc, argv, &args) || args.show_help) {
    print_help();
    return args.show_help ? 0 : 1;
  }

  // Validate required arguments
  if (!args.pubkey || !args.privkey || !args.tether) {
    fprintf(stderr,
            "Error: --pubkey, --privkey, and --tether are required.\n\n");
    print_help();
    return 1;
  }

  init_tether_regions(args.tether);

  if (enet_initialize() != 0) {
    std::cerr << "An error occurred while initializing ENet.\n";
    return 1;
  }

  ENetAddress address{};
  if (enet_address_get_host_new(&address, (char*)args.host, strlen(args.host)) <
      0) {
    std::cerr << "An error occured while creating host address.\n";
    return 1;
  }

  ENetHost* host = enet_host_create(&address, args.max_clients, 2, 0, 0);
  if (!host) {
    std::cerr << "Failed to create ENet server host.\n";
    return 1;
  }

  std::cout << "Started a Tether server...\n";
  auto priv = readbytes(args.privkey);
  auto pub = readbytes(args.pubkey);
  TetherServer server(host, priv, pub);
  server.serve();  // Blocking call.

  enet_deinitialize();
  return 0;
}