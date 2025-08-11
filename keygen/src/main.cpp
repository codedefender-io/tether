#include <Windows.h>
#include <bcrypt.h>
#include <stdint.h>
#include <stdio.h>
#include <fstream>

#include "monocypher.h"
#pragma comment(lib, "bcrypt.lib")

static void secure_random(uint8_t* buf, size_t len) {
    NTSTATUS status = BCryptGenRandom(
        NULL,
        buf,
        (ULONG)len,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    if (status != 0) {
        fprintf(stderr, "BCryptGenRandom failed (0x%08lx)\n", status);
        exit(1);
    }
}

static void write_file(const char* name, uint8_t* buf, size_t len) {
    std::ofstream ofs(name, std::ios::binary);
    ofs.write((const char*)buf, len);
    ofs.close();
}

int main(void) {
    uint8_t secret_key[32];
    uint8_t public_key[32];

    secure_random(secret_key, 32);
    crypto_x25519_public_key(public_key, secret_key);

    printf("Secret key: ");
    for (size_t i = 0; i < 32; i++) printf("%02x", secret_key[i]);
    printf("\n");

    printf("Public key: ");
    for (size_t i = 0; i < 32; i++) printf("%02x", public_key[i]);
    printf("\n");

    write_file("public-key.bin", public_key, sizeof(public_key));
    write_file("private-key.bin", secret_key, sizeof(secret_key));
    crypto_wipe(secret_key, 32);
    return 0;
}
