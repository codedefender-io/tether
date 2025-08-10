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
  if (pckt->version != VERSION) {
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
  KeyExchangePacket pckt{};
  pckt.type = TetherPacketType::KeyExchange;
  pckt.version = VERSION;
  memcpy(pckt.pub, _pub.data(), _pub.size());

  ENetPacket* packet =
      enet_packet_create(&pckt, sizeof(pckt), ENET_PACKET_FLAG_RELIABLE);

  if (packet == nullptr) {
    std::cerr << "Failed to allocate packet!";
    exit(-1);
  }

  if (enet_peer_send(peer, 0, packet) < 0) {
    // TODO log error: failed to send packet
    enet_packet_destroy(packet);
    enet_peer_disconnect(peer, 0);
  }
}

int handle_region_execution(ENetPeer* peer,
                            ENetPacket* packet,
                            __tether_registers* regs) {
  // Check peer has session state
  if (peer->data == nullptr) {
    return -1;
  }

  // Need at least nonce + MAC + ciphertext-of-regs
  if (packet->dataLength < (NONCE_SIZE + MAC_SIZE + regs_size)) {
    // too small, malformed
    // TODO log error
    return -1;
  }

  uint8_t* wire = packet->data;
  size_t wire_len = packet->dataLength;

  // Extract fields
  const uint8_t* nonce = wire;                       // wire[0 .. 23]
  const uint8_t* ciphertext = wire + NONCE_SIZE;     // wire[24 .. 24+ct-1]
  size_t ct_len = wire_len - NONCE_SIZE - MAC_SIZE;  // should equal regs_size
  const uint8_t* mac = wire + NONCE_SIZE + ct_len;   // last 16 bytes

  if (ct_len != regs_size) {
    // unexpected ciphertext length
    return -1;
  }

  TetherPeer* peer_state = reinterpret_cast<TetherPeer*>(peer->data);

  // Prepare buffer for decrypted registers
  memset(regs, 0, sizeof(regs));

  // Use packet type as associated data (bind packet type into
  // authentication)
  TetherPacketType ad_type = TetherPacketType::ExecuteTetherRegion;
  const uint8_t* ad = reinterpret_cast<const uint8_t*>(&ad_type);
  size_t ad_len = sizeof(ad_type);

  // Decrypt & verify MAC
  int ok =
      crypto_aead_unlock(reinterpret_cast<uint8_t*>(regs),  // output plaintext
                         mac,                               // tag (pointer)
                         peer_state->session,               // key (32 bytes)
                         nonce,                             // nonce (24 bytes)
                         ad, ad_len,         // associated data (type)
                         ciphertext, ct_len  // ciphertext + length
      );

  if (ok != 0) {
    // Authentication failed, bad MAC -> drop client
    return -1;
  }

  // Sanitize & execute.
  sanitize_eflags(regs);
  __execute_tether_region(regs->token, regs, __tether_regions);
  return 0;
}

void TetherServer::handle_receive(ENetPeer* peer, ENetPacket* packet) {
  if (packet->dataLength < sizeof(TetherPacketType)) {
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
          // Encrypt new register state and send it back to the client.

      }
      break;
    }
    default:
      // unknown packet type.. drop
      enet_peer_disconnect(peer, 0);
      break;
  }
}

void TetherServer::serve() {
  ENetEvent event;
  while (true) {
    while (enet_host_service(_host, &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
          handle_connect(event.peer);
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
  }
  enet_host_destroy(_host);
}