#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <Windows.h>
#include <bcrypt.h>
#include <vector>

#include <spdlog/spdlog.h>
#include "eflags.hpp"
#include "enet.h"
#include "exec.hpp"
#include "format"
#include "iostream"
#include "monocypher.h"
#include "tether.hpp"

#define TETHER_VERSION 0x00000001

constexpr size_t NONCE_SIZE = 24;
constexpr size_t MAC_SIZE = 16;
constexpr size_t regs_size = sizeof(__tether_registers);

enum TetherPacketType {
  /// <summary>
  /// Change public/private keys, derive shared secret, use that shared secret
  /// to
  /// encrypt CPU context state in futher networking packets.
  /// </summary>
  KeyExchange,
  /// <summary>
  /// Execute specified tether region using specified register context.
  /// </summary>
  ExecuteTetherRegion,
  /// <summary>
  /// A packet from the server containing the new CPU state.
  /// </summary>
  CPUStateTransfer,
};

struct KeyExchangePacket {
  /// <summary>
  /// Ignore this, should be "KeyExchange"
  /// </summary>
  TetherPacketType type;
  /// <summary>
  /// Version control system
  /// </summary>
  uint32_t version;
  /// <summary>
  /// Public key for x25519
  /// </summary>
  uint8_t pub[32];
};

#pragma pack(push, 1)
struct CPUStateTransferPacket {
  TetherPacketType type;
  uint8_t nonce[NONCE_SIZE];
  uint8_t ciphertext[regs_size];
  uint8_t mac[MAC_SIZE];
};
#pragma pack(pop)

struct TetherPeer {
  /// <summary>
  /// Session key bytes
  /// </summary>
  uint8_t session[32];
};

class TetherServer {
  ENetHost* _host;
  std::vector<uint8_t> _priv;
  std::vector<uint8_t> _pub;

 public:
  TetherServer(ENetHost* host,
               std::vector<uint8_t>& priv,
               std::vector<uint8_t>& pub)
      : _host(host), _priv(priv), _pub(pub) {}
  ~TetherServer() { enet_host_destroy(_host); }
  void serve();

 private:
  void handle_key_exchange(ENetPeer* peer, const KeyExchangePacket* pckt);
  void handle_receive(ENetPeer* peer, ENetPacket* packet);
  void handle_disconnect(ENetPeer* peer);
  void handle_connect(ENetPeer* peer);
  void handle_state_transfer(ENetPeer* peer, __tether_registers* regs);
  int handle_region_execution(ENetPeer* peer,
                              ENetPacket* packet,
                              __tether_registers* regs);
  static void secure_random(uint8_t* buf, size_t len);
};