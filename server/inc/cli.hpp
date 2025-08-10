#pragma once
#include <cargs.h>
#include <stdlib.h>

typedef struct {
  const char* host;
  int port;
  unsigned max_clients;
  const char* pubkey;
  const char* privkey;
  const char* tether;
  bool show_help;
} cli_args_t;

void print_help(void);
bool parse_args(int argc, char** argv, cli_args_t* args);