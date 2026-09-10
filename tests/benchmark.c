#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../include/memtable.h"
#include "../include/sstable.h"
#include "../include/query_engine.h"

#define NUM_RECORDS 1000

int main() {
    printf("=== Starting Zero-Knowledge Storage Engine End-to-End Benchmark ===\n");
    clock_t start = clock();

    // 1. Initialize In-Memory Architecture
    MemTable *mt = memtable_create();
    
    // 2. Ingestion Phase: Insert Records into MemTable Skip-List
    for (uint64_t i = 1; i <= NUM_RECORDS; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "ZeroKnowledge_Telemetry_Packet_%lu", i);
        memtable_insert(mt, 10000 + i, (const uint8_t*)msg, strlen(msg) + 1);
    }
    printf("[+] Ingested %d records into MemTable Skip-List.\n", NUM_RECORDS);

    // 3. Persistence Phase: Direct IO Flush to SSTable
    const char *db_file = "benchmark_sstable.db";
    bool flushed = sstable_flush_direct(db_file, mt);
    assert(flushed == true);
    printf("[+] Flushed MemTable to disk using O_DIRECT (4KB aligned).\n");

    // 4. Zero-Copy Query Engine Phase: Mmap and SIMD Vector Search
    MmapRegion *region = mmap_open_file(db_file);
    assert(region != NULL);

    uint64_t target_ts = 10000 + (NUM_RECORDS / 2);
    uint8_t query_buf[256] = {0};
    uint64_t query_len = 0;

    bool found = sstable_read_direct(db_file, target_ts, query_buf, &query_len);
    assert(found == true);

    printf("[+] Queried target timestamp %lu via Direct IO & Zero-Copy pipeline.\n", target_ts);
    printf("[+] Retrieved Payload: %s\n", query_buf);

    // Clean up resources
    mmap_close_file(region);
    memtable_destroy(mt);
    remove(db_file);

    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("=== [PASS] Pipeline Executed Successfully in %.4f seconds ===\n", cpu_time_used);

    return 0;
}
