#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/query_engine.h"

int main() {
    uint64_t timestamps[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    
    // 1. Test SIMD Search Engine
    int64_t idx = simd_find_timestamp(timestamps, 8, 600);
    assert(idx == 5);

    // 2. Test Mmap Zero-Copy File Reading
    const char *tmp_file = "mmap_test.bin";
    int fd = open(tmp_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, timestamps, sizeof(timestamps));
    close(fd);

    MmapRegion *region = mmap_open_file(tmp_file);
    assert(region != NULL);
    assert(region->size == sizeof(timestamps));

    uint64_t *mapped_ts = (uint64_t*)region->mmap_ptr;
    int64_t simd_idx = simd_find_timestamp(mapped_ts, 8, 400);
    assert(simd_idx == 3);

    mmap_close_file(region);
    remove(tmp_file);

    printf("[PASS] Step 4 Baseline: Zero-Copy Mmap & AVX2 SIMD Search Verified.\n");
    return 0;
}
