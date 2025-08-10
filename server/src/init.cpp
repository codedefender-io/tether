#include <init.hpp>

void init_tether_regions(const char* file_path) {
  std::ifstream file(file_path, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file: " << file_path << "\n";
    return;
  }

  // Read all data into buffer
  std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
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

  // If/when adding linux support to this server, need to rewrite this.
  // https://music.youtube.com/watch?v=2uJ7hsdEocg
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

    __tether_regions[i] = reinterpret_cast<fn_tether_region>(insts);
    cursor = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(cursor) +
                                         inst_buffer_len);
  }
}