#include "protocol.h"
#include <string.h>

// Software/Intrinsic CRC-32C implementation
uint32_t calculate_crc32c(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0x82F63B78 & (-(crc & 1)));
        }
    }
    return ~crc;
}

size_t protocol_encode_packet(uint8_t *buf, const TelemetryPacket *pkt) {
    size_t offset = 0;
    memcpy(buf + offset, &pkt->timestamp, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    memcpy(buf + offset, &pkt->service_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buf + offset, &pkt->target_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buf + offset, &pkt->latency, sizeof(double));
    offset += sizeof(double);

    uint32_t crc = calculate_crc32c(buf, offset);
    memcpy(buf + offset, &crc, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return offset;
}

bool protocol_decode_packet(const uint8_t *buf, size_t len, TelemetryPacket *out_pkt) {
    if (len < sizeof(uint64_t) + sizeof(uint32_t)*2 + sizeof(double) + sizeof(uint32_t)) {
        return false;
    }

    size_t data_len = len - sizeof(uint32_t);
    uint32_t expected_crc = calculate_crc32c(buf, data_len);
    
    uint32_t actual_crc;
    memcpy(&actual_crc, buf + data_len, sizeof(uint32_t));

    if (expected_crc != actual_crc) return false;

    size_t offset = 0;
    memcpy(&out_pkt->timestamp, buf + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    memcpy(&out_pkt->service_id, buf + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&out_pkt->target_id, buf + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&out_pkt->latency, buf + offset, sizeof(double));
    offset += sizeof(double);

    out_pkt->crc32 = actual_crc;
    return true;
}
