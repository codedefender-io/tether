#include <winsock2.h>
#define ENET_IMPLEMENTATION
#include <enet.h>

#include <Windows.h>
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
#include <tether.hpp>
#include <server.hpp>

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
  TetherServer server(host);
  server.serve(); // Blocking call.

  enet_deinitialize();
  return 0;
}