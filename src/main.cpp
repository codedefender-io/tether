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
  uint64_t rflags;
  uint64_t token;
};

using fn_tether_region = void (*)(unsigned, __tether_packet*);
extern "C" fn_tether_region* __tether_regions = nullptr;

extern "C" __declspec(naked) void execute_tether_region(unsigned token,
                                                        __tether_packet* pckt,
                                                        fn_tether_region* handlers) {
  __asm {
    .seh_proc FUNC
    FUNC:
        PUSHFQ
        PUSH RDX

        ; Put the address of the tether handler on top of the stack
        MOV RCX, [R8+RCX*8]
        PUSH RCX

        ; Swap GPR registers
        MOV RAX, RDX
        XCHG R15, [RAX+0x00]
        XCHG R14, [RAX+0x08]
        XCHG R13, [RAX+0x10]
        XCHG R12, [RAX+0x18]
        XCHG R11, [RAX+0x20]
        XCHG R10, [RAX+0x28]
        XCHG R9,  [RAX+0x30]
        XCHG R8,  [RAX+0x38]
        XCHG RDI, [RAX+0x40]
        XCHG RSI, [RAX+0x48]
        XCHG RBP, [RAX+0x50]
        XCHG RBX, [RAX+0x58]
        XCHG RDX, [RAX+0x60]
        XCHG RCX, [RAX+0x68]

        ; Load flags
        PUSH [RAX+0x78]
        POPFQ

        XCHG RAX, [RAX+0x70]

        ; Top of the stack is the tether handler
        CALL [RSP]

        ; Save RAX on the stack and exchange it for the cpu context ptr
        XCHG RAX, [RSP+0x8]

        XCHG [RAX+0x00], R15
        XCHG [RAX+0x08], R14
        XCHG [RAX+0x10], R13
        XCHG [RAX+0x18], R12
        XCHG [RAX+0x20], R11
        XCHG [RAX+0x28], R10
        XCHG [RAX+0x30], R9
        XCHG [RAX+0x38], R8
        XCHG [RAX+0x40], RDI
        XCHG [RAX+0x48], RSI
        XCHG [RAX+0x50], RBP
        XCHG [RAX+0x58], RBX
        XCHG [RAX+0x60], RDX
        XCHG [RAX+0x68], RCX

        ; Save RAX now.
        MOV RCX, [RSP+0x8]
        MOV [RAX+0x70], RCX

        ; Save flags from tether handler
        PUSHFQ
        POP QWORD PTR [RAX+0x78]

        POP RCX
        POP RDX
        POPFQ
        RET
    .seh_endproc
  }
}

void init_tether_regions(const char* file) {
  FILE* fp = fopen(file, "rb");
  if (!fp) {
    perror("fopen");
    return;
  }

  // Seek to end to determine size
  if (fseek(fp, 0, SEEK_END) != 0) {
    perror("fseek");
    fclose(fp);
    return;
  }

  long size = ftell(fp);
  if (size < 0) {
    perror("ftell");
    fclose(fp);
    return;
  }

  rewind(fp);

  char* buffer = (char*)malloc(size);
  if (!buffer) {
    perror("malloc");
    fclose(fp);
    return;
  }

  // Read entire file
  size_t read = fread(buffer, 1, size, fp);
  if (read != (size_t)size) {
    fprintf(stderr, "fread: expected %ld bytes, got %zu\n", size, read);
    free(buffer);
    fclose(fp);
    return;
  }

  uint32_t* cursor = (uint32_t*)buffer;
  uint32_t regions = *cursor;
  cursor++;

  printf("Loading regions: %d\n", regions);

  __tether_regions =
      (fn_tether_region*)malloc(regions * sizeof(fn_tether_region));

  if (!__tether_regions) {
    printf("Failed to allocate memory for __tether_regions!");
    return;
  }

  for (unsigned i = 0; i < regions; i++) {
    uint32_t token = *cursor;
    cursor++;
    uint32_t inst_buffer_len = *cursor;
    cursor++;

    uint8_t* insts =
        (uint8_t*)VirtualAlloc(NULL, inst_buffer_len + 1,  // Add 1 for RET
                               MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!insts) {
      printf("Failed to allocate memory for instructions!");
      return;
    }

    memcpy(insts, cursor, inst_buffer_len);
    insts[inst_buffer_len] = 0xC3;  // Add a RET

    DWORD old;
    VirtualProtect(insts, inst_buffer_len + 1, PAGE_EXECUTE, &old);

    printf("Token: %d, instruction buffer size: %d ptr = %p\n", token,
           inst_buffer_len, insts);

    __tether_regions[i] = (fn_tether_region)insts;
    cursor = (uint32_t*)(((uint8_t*)cursor) + inst_buffer_len);
  }

  fclose(fp);
  free(buffer);
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    printf("Please provide a path to a tether file!");
    return 1;
  }

  init_tether_regions(argv[1]);

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
          printf(
              " RBP: %016llX   RSI: %016llX   RDI: %016llX   \n\nRFLAGS: "
              "%016llX\n",
              pckt->grps.rbp, pckt->grps.rsi, pckt->grps.rdi, pckt->rflags);
          printf("------------------------\n");
          printf("TOKEN: %016llX\n", pckt->token);

          // Execute the tether handler now.
          execute_tether_region(pckt->token, pckt, __tether_regions);

          // Create an enet packet with the new cpu state
          ENetPacket* packet = enet_packet_create(pckt, sizeof(__tether_packet),
                                 ENET_PACKET_FLAG_RELIABLE);

          if (enet_peer_send(event.peer, 0, packet) < 0) {
            // If it failed to send we need to destory it.
            enet_packet_destroy(packet);
          }

          enet_peer_disconnect(event.peer, 0);
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