#include <stdio.h>
#include <assert.h>
#include "../include/protocol.h"

int main() {
    uint8_t buffer[10];
    uint64_t original_value = 10663;
    uint64_t decoded_value = 0;

    size_t encoded_len = encode_varint(original_value, buffer);
    size_t decoded_len = decode_varint(buffer, &decoded_value);

    assert(encoded_len == decoded_len);
    assert(decoded_value == original_value);

    printf("[PASS] Step 1 Baseline: Varint Codec Verified (%lu == %lu).\n", decoded_value, original_value);
    return 0;
}
