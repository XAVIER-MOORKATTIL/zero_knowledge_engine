#include "memtable.h"
#include <stdlib.h>
#include <string.h>

static int random_level(void) {
    int lvl = 1;
    while ((rand() & 0xFFFF) < (0.5 * 0xFFFF) && lvl < MAX_LEVEL) {
        lvl++;
    }
    return lvl;
}

MemTable* memtable_create(void) {
    MemTable *mt = (MemTable*)malloc(sizeof(MemTable));
    mt->head = (SkipNode*)calloc(1, sizeof(SkipNode));
    mt->level = 1;
    mt->size_bytes = 0;
    return mt;
}

bool memtable_insert(MemTable *mt, uint64_t timestamp, const uint8_t *payload, uint64_t len) {
    SkipNode *update[MAX_LEVEL];
    SkipNode *current = mt->head;

    for (int i = mt->level - 1; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->timestamp < timestamp) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    int lvl = random_level();
    if (lvl > mt->level) {
        for (int i = mt->level; i < lvl; i++) {
            update[i] = mt->head;
        }
        mt->level = lvl;
    }

    SkipNode *new_node = (SkipNode*)calloc(1, sizeof(SkipNode));
    new_node->timestamp = timestamp;
    new_node->payload_len = len;
    new_node->payload = (uint8_t*)malloc(len);
    memcpy(new_node->payload, payload, len);

    for (int i = 0; i < lvl; i++) {
        do {
            new_node->forward[i] = update[i]->forward[i];
        } while (!__atomic_compare_exchange_n(
            &update[i]->forward[i],
            &new_node->forward[i],
            new_node,
            true, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
    }

    __atomic_fetch_add(&mt->size_bytes, len + sizeof(SkipNode), __ATOMIC_RELAXED);
    return true;
}

void memtable_destroy(MemTable *mt) {
    if (!mt) return;
    SkipNode *current = mt->head->forward[0];
    while (current != NULL) {
        SkipNode *next = current->forward[0];
        if (current->payload) free(current->payload);
        free(current);
        current = next;
    }
    free(mt->head);
    free(mt);
}
