#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_PACKET_SIZE 1024

typedef struct {
    uint64_t timestamp;
    uint32_t service_id;
    uint32_t target_id;
    double latency;
    uint32_t crc32;
} TelemetryPacket;

uint32_t calculate_crc32c(const uint8_t *data, size_t length);
size_t protocol_encode_packet(uint8_t *buf, const TelemetryPacket *pkt);
bool protocol_decode_packet(const uint8_t *buf, size_t len, TelemetryPacket *out_pkt);

#endif
