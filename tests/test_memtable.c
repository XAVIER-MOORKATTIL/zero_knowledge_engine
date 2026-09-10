#include <stdio.h>
#include <assert.h>
#include "../include/memtable.h"

int main() {
    MemTable *mt = memtable_create();

    uint8_t data1[] = "Telemetry_Event_Alpha";
    uint8_t data2[] = "Telemetry_Event_Beta";

    memtable_insert(mt, 10001, data1, sizeof(data1));
    memtable_insert(mt, 10000, data2, sizeof(data2));

    assert(mt->head->forward[0]->timestamp == 10000);
    assert(mt->head->forward[0]->forward[0]->timestamp == 10001);

    memtable_destroy(mt);
    printf("[PASS] Step 2 Baseline: Concurrent MemTable Skip-List Invariance Verified.\n");
    return 0;
}
