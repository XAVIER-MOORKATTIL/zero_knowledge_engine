#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "sstable.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool sstable_flush_direct(const char *filepath, MemTable *mt) {
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT | O_SYNC, 0644);
    if (fd < 0) {
        perror("Failed to open file with O_DIRECT");
        return false;
    }

    // Calculate total raw bytes needed
    size_t raw_bytes = 0;
    SkipNode *curr = mt->head->forward[0];
    while (curr != NULL) {
        raw_bytes += sizeof(uint64_t) * 2 + curr->payload_len;
        curr = curr->forward[0];
    }

    // Align buffer size to next 4KB boundary
    size_t total_aligned = ((raw_bytes + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT;
    if (total_aligned == 0) total_aligned = ALIGNMENT;

    void *aligned_buf = NULL;
    if (posix_memalign(&aligned_buf, ALIGNMENT, total_aligned) != 0) {
        close(fd);
        return false;
    }
    memset(aligned_buf, 0, total_aligned);

    curr = mt->head->forward[0];
    uint8_t *ptr = (uint8_t*)aligned_buf;
    size_t offset = 0;

    while (curr != NULL) {
        memcpy(ptr + offset, &curr->timestamp, sizeof(uint64_t));
        offset += sizeof(uint64_t);

        memcpy(ptr + offset, &curr->payload_len, sizeof(uint64_t));
        offset += sizeof(uint64_t);

        memcpy(ptr + offset, curr->payload, curr->payload_len);
        offset += curr->payload_len;

        curr = curr->forward[0];
    }

    ssize_t ret = write(fd, aligned_buf, total_aligned);
    free(aligned_buf);
    close(fd);

    return ret == (ssize_t)total_aligned;
}

bool sstable_read_direct(const char *filepath, uint64_t timestamp, uint8_t *out_buffer, uint64_t *out_len) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return false;

    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (file_size <= 0) {
        close(fd);
        return false;
    }

    size_t aligned_size = ((file_size + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT;
    void *aligned_buf = NULL;
    if (posix_memalign(&aligned_buf, ALIGNMENT, aligned_size) != 0) {
        close(fd);
        return false;
    }

    ssize_t ret = read(fd, aligned_buf, aligned_size);
    close(fd);
    if (ret <= 0) {
        free(aligned_buf);
        return false;
    }

    uint8_t *ptr = (uint8_t*)aligned_buf;
    size_t offset = 0;

    while (offset < (size_t)ret) {
        uint64_t ts = *(uint64_t*)(ptr + offset);
        if (ts == 0 && offset > 0) break;
        offset += sizeof(uint64_t);

        uint64_t len = *(uint64_t*)(ptr + offset);
        offset += sizeof(uint64_t);

        if (ts == timestamp) {
            memcpy(out_buffer, ptr + offset, len);
            *out_len = len;
            free(aligned_buf);
            return true;
        }
        offset += len;
    }

    free(aligned_buf);
    return false;
}
