#include <cli.hpp>

void print_help(void) {
  printf("Tether Server\n");
  printf("Usage:\n");
  printf("  server [OPTIONS]\n\n");
  printf("Options:\n");
  printf("  -h, --host HOST        Host address to bind (default: 0.0.0.0)\n");
  printf("  -p, --port PORT        Port number to bind (default: 1234)\n");
  printf(
      "  -c, --max-clients      Max number of connections the server supports "
      "at once (default 128)\n");
  printf("      --pubkey FILE      Path to public key file (required)\n");
  printf("      --privkey FILE     Path to private key file (required)\n");
  printf("      --tether FILE      Path to tether file (required)\n");
  printf("      --help             Show this help message\n");
}

bool parse_args(int argc, char** argv, cli_args_t* args) {
  static struct cag_option options[] = {
      {.identifier = 'h',
       .access_letters = "h",
       .access_name = "host",
       .value_name = "HOST",
       .description = "Host address to bind (default: 0.0.0.0)"},
      {.identifier = 'p',
       .access_letters = "p",
       .access_name = "port",
       .value_name = "PORT",
       .description = "Port number to bind (default: 1234)"},
      {.identifier = 'c',
       .access_letters = "p",
       .access_name = "max-clients",
       .value_name = "MAX-CLIENTS",
       .description = "Max number of connections the server supports at once"},
      {.identifier = 'u',
       .access_letters = NULL,
       .access_name = "pubkey",
       .value_name = "FILE",
       .description = "Path to public key file (required)"},
      {.identifier = 'r',
       .access_letters = NULL,
       .access_name = "privkey",
       .value_name = "FILE",
       .description = "Path to private key file (required)"},
      {.identifier = 't',
       .access_letters = NULL,
       .access_name = "tether",
       .value_name = "FILE",
       .description = "Path to tether file (required)"},
      {.identifier = '?',
       .access_letters = NULL,
       .access_name = "help",
       .value_name = NULL,
       .description = "Show this help message"}};

  // Defaults
  args->host = "localhost";
  args->port = 1234;
  args->max_clients = 128;
  args->pubkey = NULL;
  args->privkey = NULL;
  args->tether = NULL;
  args->show_help = false;

  cag_option_context ctx;
  cag_option_prepare(&ctx, options, CAG_ARRAY_SIZE(options), argc, argv);

  while (cag_option_fetch(&ctx)) {
    switch (cag_option_get_identifier(&ctx)) {
      case 'h':
        args->host = cag_option_get_value(&ctx);
        break;
      case 'p':
        args->port = atoi(cag_option_get_value(&ctx));
        break;
      case 'c':
        args->max_clients = atoi(cag_option_get_value(&ctx));
        break;
      case 'u':
        args->pubkey = cag_option_get_value(&ctx);
        break;
      case 'r':
        args->privkey = cag_option_get_value(&ctx);
        break;
      case 't':
        args->tether = cag_option_get_value(&ctx);
        break;
      case '?':
        args->show_help = true;
        break;
      default:
        return false;
    }
  }

  return true;
}