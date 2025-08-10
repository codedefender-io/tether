#pragma once
#include "tether.hpp"

class TetherServer {
  ENetHost* _host;
 public:
  TetherServer(ENetHost* host) : _host(host) {}
  ~TetherServer() { enet_host_destroy(_host); }
  void serve();
};