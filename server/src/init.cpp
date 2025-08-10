#include <Windows.h>

#include <init.hpp>
#include <fstream>
#include <vector>
#include <format>
#include <iostream>

#include <tether.hpp>

void init_tether_regions(const std::string& file_path) {
  std::ifstream file(file_path, std::ios::binary);
  if (!file) {
    std::cerr << std::format("Failed to open file: {}\n", file_path);
    return;
  }

  // Read all data into buffer
  std::vector<std::byte> buffer(std::istreambuf_iterator<char>(file), {});
  if (buffer.empty()) {
    std::cerr << "File is empty or failed to read.\n";
    return;
  }

  auto cursor = reinterpret_cast<uint32_t*>(buffer.data());
  uint32_t regions = *cursor++;

  __tether_regions = static_cast<fn_tether_region*>(
      std::malloc(regions * sizeof(fn_tether_region)));

  if (!__tether_regions) {
    std::cerr << "Failed to allocate memory for __tether_regions!\n";
    return;
  }

  for (unsigned i = 0; i < regions; i++) {
    uint32_t token = *cursor++;
    uint32_t inst_buffer_len = *cursor++;

    auto insts = static_cast<uint8_t*>(
        VirtualAlloc(nullptr, inst_buffer_len + 1, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE));
    if (!insts) {
      std::cerr << "Failed to allocate memory for instructions!\n";
      return;
    }

    std::memcpy(insts, cursor, inst_buffer_len);
    insts[inst_buffer_len] = 0xC3;  // RET

    DWORD old;
    VirtualProtect(insts, inst_buffer_len + 1, PAGE_EXECUTE, &old);

    std::cout << std::format(
        "Token: {}, instruction buffer size: {}, ptr = {}\n", token,
        inst_buffer_len, static_cast<void*>(insts));

    __tether_regions[i] = reinterpret_cast<fn_tether_region>(insts);
    cursor = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(cursor) +
                                         inst_buffer_len);
  }
}