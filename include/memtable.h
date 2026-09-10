#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_LEVEL 16

typedef struct SkipNode {
    uint64_t timestamp;
    uint64_t payload_len;
    uint8_t *payload;
    struct SkipNode *forward[MAX_LEVEL];
} SkipNode;

typedef struct {
    SkipNode *head;
    int level;
    size_t size_bytes;
} MemTable;

MemTable* memtable_create(void);
bool memtable_insert(MemTable *mt, uint64_t timestamp, const uint8_t *payload, uint64_t len);
void memtable_destroy(MemTable *mt);

#endif
