#include <winsock2.h>
#define ENET_IMPLEMENTATION
#include <enet.h>

#include <Windows.h>
#include <stdio.h>

#define MAX_CLIENTS 32

struct __tether_packet {
  struct __grps {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;
  } grps;
  uint64_t token;
};

int main(int argc, char** argv) {
  if (enet_initialize() != 0) {
    printf("An error occurred while initializing ENet.\n");
    return 1;
  }

  ENetAddress address = {0};
  address.host = ENET_HOST_ANY; /* Bind the server to the default localhost. */
  address.port = 1234;          /* Bind the server to port 1234. */

  /* create a server */
  ENetHost* server = enet_host_create(&address, MAX_CLIENTS, 1, 0, 0);

  if (server == NULL) {
    printf("An error occurred while trying to create an ENet server host.\n");
    return 1;
  }

  printf("Started a Tether server...\n");
  ENetEvent event;

  while (true) {
    while (enet_host_service(server, &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
          printf("A new client connected from %x:%u.\n",
                 event.peer->address.host, event.peer->address.port);
          break;
        case ENET_EVENT_TYPE_RECEIVE: {
          auto pckt = reinterpret_cast<__tether_packet*>(event.packet->data);
          printf("==== TETHER PACKET ====\n");
          printf(" RAX: %016llX   RBX: %016llX   RCX: %016llX   RDX: %016llX\n",
                 pckt->grps.rax, pckt->grps.rbx, pckt->grps.rcx,
                 pckt->grps.rdx);
          printf("  R8: %016llX    R9: %016llX   R10: %016llX   R11: %016llX\n",
                 pckt->grps.r8, pckt->grps.r9, pckt->grps.r10, pckt->grps.r11);
          printf(" R12: %016llX   R13: %016llX   R14: %016llX   R15: %016llX\n",
                 pckt->grps.r12, pckt->grps.r13, pckt->grps.r14,
                 pckt->grps.r15);
          printf(" RBP: %016llX   RSI: %016llX   RDI: %016llX\n",
                 pckt->grps.rbp, pckt->grps.rsi, pckt->grps.rdi);
          printf("------------------------\n");
          printf("TOKEN: %016llX\n", pckt->token);
          enet_packet_destroy(event.packet);
          break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
          printf("Client %x:%u disconnected.\n", event.peer->address.host,
                 event.peer->address.port);
          break;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          printf("Client %x:%u timeout!\n", event.peer->address.host,
                 event.peer->address.port);
          break;
        case ENET_EVENT_TYPE_NONE:
          break;
      }
    }
  }

  enet_host_destroy(server);
  enet_deinitialize();
  return 0;
}