#ifndef SSTABLE_H
#define SSTABLE_H

#include "memtable.h"
#include <stdbool.h>

#define ALIGNMENT 4096

bool sstable_flush_direct(const char *filepath, MemTable *mt);
bool sstable_read_direct(const char *filepath, uint64_t timestamp, uint8_t *out_buffer, uint64_t *out_len);

#endif
