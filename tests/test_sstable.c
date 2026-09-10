#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/memtable.h"
#include "../include/sstable.h"

int main() {
    MemTable *mt = memtable_create();
    uint8_t payload[] = "ZeroKnowledge_DirectIO_Payload";
    uint64_t target_ts = 20002;

    memtable_insert(mt, 20001, (uint8_t*)"Event1", 6);
    memtable_insert(mt, target_ts, payload, sizeof(payload));

    const char *test_file = "sstable_test.db";
    bool flushed = sstable_flush_direct(test_file, mt);
    assert(flushed == true);

    uint8_t read_buf[128] = {0};
    uint64_t read_len = 0;
    bool found = sstable_read_direct(test_file, target_ts, read_buf, &read_len);

    assert(found == true);
    assert(read_len == sizeof(payload));
    assert(memcmp(read_buf, payload, read_len) == 0);

    memtable_destroy(mt);
    remove(test_file);

    printf("[PASS] Step 3 Baseline: Direct IO (O_DIRECT) SSTable Disk Flush Verified.\n");
    return 0;
}
