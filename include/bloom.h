#ifndef BLOOM_H
#define BLOOM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t *counts;
    size_t size_m;
    size_t num_hashes_k;
} CountingBloomFilter;

CountingBloomFilter* bloom_create(size_t expected_elements_n);
void bloom_insert(CountingBloomFilter *filter, const uint8_t *key, size_t len);
bool bloom_check(CountingBloomFilter *filter, const uint8_t *key, size_t len);
void bloom_destroy(CountingBloomFilter *filter);

#endif
