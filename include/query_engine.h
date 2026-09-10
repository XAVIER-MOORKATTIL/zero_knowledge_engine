#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int fd;
    size_t size;
    void *mmap_ptr;
} MmapRegion;

MmapRegion* mmap_open_file(const char *filepath);
void mmap_close_file(MmapRegion *region);
int64_t simd_find_timestamp(const uint64_t *timestamps, size_t count, uint64_t target);

#endif
