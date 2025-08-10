#include <format>
#include <iostream>

#include "enet.h"
#include "exec.hpp"
#include "server.hpp"
#include "sanitizer.hpp"
#include "tether.hpp"

// Main server logic, this function is blocking.
void TetherServer::serve() {
  ENetEvent event;
  while (true) {
    while (enet_host_service(_host, &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
          break;
        case ENET_EVENT_TYPE_RECEIVE: {
          auto pckt = reinterpret_cast<__tether_packet*>(event.packet->data);
          execute_tether_region(pckt->token, pckt, __tether_regions);

          ENetPacket* packet = enet_packet_create(pckt, sizeof(__tether_packet),
                                                  ENET_PACKET_FLAG_RELIABLE);

          if (enet_peer_send(event.peer, 0, packet) < 0) {
            enet_packet_destroy(packet);
          }

          enet_packet_destroy(event.packet);
          break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
          break;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
          std::cout << std::format("Client {}:{} timeout!\n",
                                   event.peer->address.host,
                                   event.peer->address.port);
          break;
        case ENET_EVENT_TYPE_NONE:
          break;
      }
    }
  }
  enet_host_destroy(_host);
}