#include "query_engine.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <emmintrin.h>

MmapRegion* mmap_open_file(const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        close(fd);
        return NULL;
    }

    void *ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    MmapRegion *region = (MmapRegion*)malloc(sizeof(MmapRegion));
    region->fd = fd;
    region->size = sb.st_size;
    region->mmap_ptr = ptr;
    return region;
}

void mmap_close_file(MmapRegion *region) {
    if (!region) return;
    munmap(region->mmap_ptr, region->size);
    close(region->fd);
    free(region);
}

// Fixed Portable SSE2 Search
int64_t simd_find_timestamp(const uint64_t *timestamps, size_t count, uint64_t target) {
    size_t i = 0;
    __m128i target_vec = _mm_set1_epi64x((long long)target);

    for (; i + 2 <= count; i += 2) {
        __m128i data_vec = _mm_loadu_si128((const __m128i*)(timestamps + i));
        __m128i cmp = _mm_cmpeq_epi32(data_vec, target_vec);
        int mask = _mm_movemask_epi8(cmp);
        
        // If any 32-bit dword matches, inspect elements in scalar loop
        if (mask != 0) {
            for (size_t j = 0; j < 2; j++) {
                if (timestamps[i + j] == target) return (int64_t)(i + j);
            }
        }
    }

    // Scalar fallback
    for (; i < count; i++) {
        if (timestamps[i] == target) return (int64_t)i;
    }

    return -1;
}
