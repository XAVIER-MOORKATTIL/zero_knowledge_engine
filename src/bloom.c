#include "bloom.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

static uint64_t fnv1a_hash(const uint8_t *key, size_t len, uint64_t seed) {
    uint64_t hash = 14695981039346656037ULL ^ seed;
    for (size_t i = 0; i < len; i++) {
        hash ^= key[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

CountingBloomFilter* bloom_create(size_t expected_elements_n) {
    CountingBloomFilter *filter = (CountingBloomFilter*)malloc(sizeof(CountingBloomFilter));
    
    // Math requirement for p = 10^-18: m/n ~ 86 bits, k = 60 hashes
    filter->size_m = expected_elements_n * 86;
    filter->num_hashes_k = 60;
    filter->counts = (uint8_t*)calloc(filter->size_m, sizeof(uint8_t));

    return filter;
}

void bloom_insert(CountingBloomFilter *filter, const uint8_t *key, size_t len) {
    for (size_t i = 0; i < filter->num_hashes_k; i++) {
        uint64_t hash = fnv1a_hash(key, len, i * 0x9E3779B97F4A7C15ULL);
        size_t idx = hash % filter->size_m;
        if (filter->counts[idx] < 255) {
            filter->counts[idx]++;
        }
    }
}

bool bloom_check(CountingBloomFilter *filter, const uint8_t *key, size_t len) {
    for (size_t i = 0; i < filter->num_hashes_k; i++) {
        uint64_t hash = fnv1a_hash(key, len, i * 0x9E3779B97F4A7C15ULL);
        size_t idx = hash % filter->size_m;
        if (filter->counts[idx] == 0) return false;
    }
    return true;
}

void bloom_destroy(CountingBloomFilter *filter) {
    if (!filter) return;
    free(filter->counts);
    free(filter);
}
