#include "server.hpp"

#pragma comment(lib, "bcrypt.lib")

void TetherServer::secure_random(uint8_t* buf, size_t len) {
  NTSTATUS status =
      BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  if (status != 0) {
    fprintf(stderr, "BCryptGenRandom failed (0x%08lx)\n", status);
    exit(1);
  }
}

void TetherServer::handle_disconnect(ENetPeer* peer) {
  // Free up client session data if any
  if (peer->data != nullptr) {
    free(peer->data);
    peer->data = nullptr;
  }
}

void TetherServer::handle_key_exchange(ENetPeer* peer,
                                       const KeyExchangePacket* pckt) {
  if (pckt->version != TETHER_VERSION) {
    spdlog::warn("Invalid peer version {}, dropping connection", pckt->version);
    enet_peer_disconnect(peer, 0);
    return;
  }

  uint8_t shared_secret[32];
  crypto_x25519(shared_secret, _priv.data(), pckt->pub);

  uint8_t kd_input[96];  // ss || pkA || pkB
  memcpy(kd_input, shared_secret, 32);
  memcpy(kd_input + 32, _pub.data(), 32);
  memcpy(kd_input + 64, pckt->pub, 32);

  TetherPeer new_peer;
  crypto_blake2b(new_peer.session, sizeof(new_peer.session), kd_input,
                 sizeof(kd_input));

  auto buff = malloc(sizeof(new_peer));
  memcpy(buff, &new_peer, sizeof(new_peer));
  peer->data = buff;
}

void TetherServer::handle_connect(ENetPeer* peer) {
  // Send newly connected peer our public key in a KeyExchangePacket
  KeyExchangePacket pckt;
  pckt.type = TetherPacketType::KeyExchange;
  pckt.version = TETHER_VERSION;
  memcpy(pckt.pub, _pub.data(), _pub.size());

  ENetPacket* packet = enet_packet_create(&pckt, sizeof(KeyExchangePacket),
                                          ENET_PACKET_FLAG_RELIABLE);

  if (packet == nullptr) {
    spdlog::critical("Failed to allocate new packet! Shutting down!");
    exit(-1);
  }

  if (enet_peer_send(peer, 0, packet) < 0) {
    spdlog::error("Failed to send public key! Disconnecting peer");
    enet_packet_destroy(packet);
    enet_peer_disconnect(peer, 0);
  }
}

void TetherServer::handle_state_transfer(ENetPeer* peer,
                                         __tether_registers* regs) {
  TetherPeer* peer_state = reinterpret_cast<TetherPeer*>(peer->data);
  CPUStateTransferPacket wire;
  wire.type = TetherPacketType::CPUStateTransfer;

  // Generate a random nonce
  secure_random(wire.nonce, NONCE_SIZE);
  const uint8_t* ad = reinterpret_cast<const uint8_t*>(&wire.type);
  size_t ad_len = sizeof(wire.type);

  crypto_aead_lock(wire.ciphertext, wire.mac, peer_state->session, wire.nonce,
                   ad, ad_len, reinterpret_cast<const uint8_t*>(regs),
                   regs_size);

  ENetPacket* pkt =
      enet_packet_create(&wire, sizeof(wire), ENET_PACKET_FLAG_RELIABLE);

  if (!pkt) {
    spdlog::error("Failed to create ENet packet for state transfer.");
    return;
  }

  if (enet_peer_send(peer, 0, pkt) < 0) {
    spdlog::error("Failed to send new cpu state!");
    enet_packet_destroy(pkt);
  }
}

int TetherServer::handle_region_execution(ENetPeer* peer,
                                          ENetPacket* packet,
                                          __tether_registers* regs) {
  if (!peer || !peer->data) {
    spdlog::error("Peer or peer->data missing, dropping connection!");
    return -1;
  }

  if (packet->dataLength < sizeof(CPUStateTransferPacket)) {
    spdlog::error("Packet too small ({} < {}), dropping connection!",
                  packet->dataLength, sizeof(CPUStateTransferPacket));
    return -1;
  }

  const CPUStateTransferPacket* wire =
      reinterpret_cast<const CPUStateTransferPacket*>(packet->data);

  if (wire->type != TetherPacketType::ExecuteTetherRegion) {
    spdlog::error("Unexpected packet type {}, dropping connection!",
                  static_cast<int>(wire->type));
    return -1;
  }

  TetherPeer* peer_state = reinterpret_cast<TetherPeer*>(peer->data);
  memset(regs, 0, sizeof(__tether_registers));

  // Decrypt & verify MAC (AD = type field)
  const uint8_t* ad = reinterpret_cast<const uint8_t*>(&wire->type);
  size_t ad_len = sizeof(wire->type);

  int ok = crypto_aead_unlock(reinterpret_cast<uint8_t*>(regs), wire->mac,
                              peer_state->session, wire->nonce, ad, ad_len,
                              wire->ciphertext, regs_size);

  if (ok != 0) {
    spdlog::critical("Bad MAC for packet! Possible tampering or wrong key.");
    return -1;
  }

  EFLAGS orig_flags = sanitize_eflags(regs);
  __execute_tether_region(regs->token, regs, __tether_regions);

  // Copy status flags back into the original EFLAG's
  EFLAGS* new_flags = reinterpret_cast<EFLAGS*>(&regs->rflags);
  orig_flags.ZeroFlag = new_flags->ZeroFlag;
  orig_flags.SignFlag = new_flags->SignFlag;
  orig_flags.ParityFlag = new_flags->ParityFlag;
  orig_flags.CarryFlag = new_flags->CarryFlag;
  orig_flags.OverflowFlag = new_flags->OverflowFlag;

  regs->rflags = orig_flags.AsUInt;
  return 0;
}

void TetherServer::handle_receive(ENetPeer* peer, ENetPacket* packet) {
  if (packet->dataLength < sizeof(TetherPacketType)) {
    spdlog::error("Packet too small, malformed.");
    enet_peer_disconnect(peer, 0);
    return;
  }

  auto type = *reinterpret_cast<TetherPacketType*>(packet->data);
  switch (type) {
    case TetherPacketType::KeyExchange: {
      if (packet->dataLength >= sizeof(KeyExchangePacket)) {
        auto pckt = reinterpret_cast<KeyExchangePacket*>(packet->data);
        handle_key_exchange(peer, pckt);
      } else {
        enet_peer_disconnect(peer, 0);
      }
      break;
    }
    case TetherPacketType::ExecuteTetherRegion: {
      __tether_registers regs;
      if (handle_region_execution(peer, packet, &regs) < 0) {
        enet_peer_disconnect(peer, 0);
      } else {
        handle_state_transfer(peer, &regs);
      }
      break;
    }
    default:
      spdlog::error("Unknown packet type, dropping connection!");
      enet_peer_disconnect(peer, 0);
      break;
  }
}

void TetherServer::serve() {
  ENetEvent event;
  while (enet_host_service(_host, &event, 1000) >= 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        if (event.data != TETHER_VERSION) {
          handle_disconnect(event.peer);
        } else {
          handle_connect(event.peer);
        }
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        handle_receive(event.peer, event.packet);
        enet_packet_destroy(event.packet);
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
      case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
        handle_disconnect(event.peer);
        break;

      case ENET_EVENT_TYPE_NONE:
        break;
    }
  }
  enet_host_destroy(_host);
}