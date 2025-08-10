#pragma
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <tether.hpp>

void init_tether_regions(const char* file_path);