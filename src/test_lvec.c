
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>

#include "lvec.h"




/* A Fast hash function implementation by Paul Hsieh
    http://www.azillionmonkeys.com/qed/hash.html
*/
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif
#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
static inline uint32_t super_fast_hash (const char *data, int len) {
    uint32_t hash = len, tmp;
    int rem;
    if (len <= 0 || data == NULL) return 0; // todo: can this be safely removed?
    rem = len & 3;
    len >>= 2;
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash;
}
// End of Fast hash function implementation


#define hash_lvec64(v) super_fast_hash((char*) v, sizeof(lvec64_t) + v->element_width * v->element_count_max)


#define TEST_STARTING printf("running test %s ", __func__)
#define TEST_PASSED printf("[ok]\n")

void test_lvec_get_segment() {
    TEST_STARTING;

    lvec_header_t head;
    head.element_width = 24;
    lvec_header_t *ptr = &head;
    
    lvec_segment_t *seg;
    
    // Test the first segment = head_ptr + sizeof head
    seg = lvec_get_segment(ptr, 0);
    assert(
        (((uint8_t*)ptr) + sizeof(lvec_header_t))
        == (uint8_t*)seg
    );
    // Test getting ptr to arbitrary segments.
    for(int i = 0; i < 10000; i++) {
        seg = lvec_get_segment(ptr, i);
        assert(
            (((uint8_t*)ptr) + sizeof(lvec_header_t) + head.element_width*LVEC_SEGMENT_SIZE*i + sizeof(lvec64_occupancy_bitmap_t)*i)
            == (uint8_t*)seg
        );
    }
    TEST_PASSED;
}


int main() {

    // Test create vector functionality
    test_lvec_get_segment();


    return 0;
}
