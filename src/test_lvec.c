
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>

#include "lvec.h"


#define TEST_STARTING printf("running test %s ", __func__)
#define TEST_PASSED printf("[ok]\n")

void test_lvec_create(void) {
    TEST_STARTING;

    lvec_header_t *v = lvec_create(24, 2, false);
    assert(v != NULL);
    assert(v->element_width == 24);
    assert(v->segment_count == 2);
    assert(v->hard_delete_slots == false);

    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_get_segment(void) {
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

void test_lvec_get_slots_count(void) {
    TEST_STARTING;
    lvec_header_t *v;
    
    v = lvec_create(24, 1, false);
    assert(v);
    assert(64*1 == lvec_get_slots_count(v));
    lvec_free(v);

    v = lvec_create(24, 2, false);
    assert(v);
    assert(64*2 == lvec_get_slots_count(v));
    lvec_free(v);

    v = lvec_create(24, 3, false);
    assert(v);
    assert(64*3 == lvec_get_slots_count(v));
    lvec_free(v);

    TEST_PASSED;
}


void test_lvec_get_segment_ix_from_slot_ix(void) {
    TEST_STARTING;
    for(int i = 0; i < 64; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 0);
    for(int i = 64; i < 128; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 1);
    for(int i = 128; i < 192; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 2);
    for(int i = 192; i < 256; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 3);
    for(int i = 256; i < 320; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 4);
    for(int i = 320; i < 384; i++) 
        assert(lvec_get_segment_ix_from_slot_ix(i) == 5);
    TEST_PASSED;
}


void test_lvec_localize_slot_ix(void) {
    TEST_STARTING;
    for(int i=0; i< 10000; i++)
        assert(lvec_localize_slot_ix(i) >= 0 && lvec_localize_slot_ix(i) < 64);

    for(int i = 0; i < 64; i++) 
        assert(lvec_localize_slot_ix(i) == i);
    for(int i = 64; i < 128; i++) 
        assert(lvec_localize_slot_ix(i) == (i - 64));
    for(int i = 128; i < 192; i++)
        assert(lvec_localize_slot_ix(i) == (i - 128));
    for(int i = 192; i < 256; i++)
        assert(lvec_localize_slot_ix(i) == (i - 192));
    for(int i = 256; i < 320; i++)
        assert(lvec_localize_slot_ix(i) == (i - 256));
    for(int i = 320; i < 384; i++)
        assert(lvec_localize_slot_ix(i) == (i - 320));
    TEST_PASSED;
}


void test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment(void) {
    TEST_STARTING;
    TEST_PASSED;

}


int main() {
    test_lvec_create();
    test_lvec_get_segment();
    test_lvec_get_slots_count();
    test_lvec_get_segment_ix_from_slot_ix();
    test_lvec_localize_slot_ix();

    test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment();


    return 0;
}
