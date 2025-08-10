#include "sanitizer.hpp"

void sanitize(__tether_packet* pckt) {
  auto eflags = reinterpret_cast<EFLAGS*>(pckt->rflags);
  eflags->TrapFlag = false;
}