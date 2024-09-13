
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

void test_lvec_get_data_ptr(void) {
    TEST_STARTING;
    lvec_header_t *v = lvec_create(sizeof(int), 1, false);
    assert(v);

    lvec_segment_t *seg0 = lvec_get_segment(v, 0);
    for(int i=0; i<64; i++ ){
        uint8_t* ptr = lvec_get_data_ptr(v, i);
        assert(ptr);
        assert(ptr == seg0->data + i*sizeof(int));
    }

    lvec_segment_t *seg1 = lvec_get_segment(v, 1);
    for(int i=64; i<127; i++ ){
        uint8_t* ptr = lvec_get_data_ptr(v, i);
        assert(ptr);
        assert(ptr == seg1->data + lvec_localize_slot_ix(i)*sizeof(int));
    }

    lvec_free(v);
    TEST_PASSED;
}


void test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment(void) {
    TEST_STARTING;
    lvec_header_t *v = lvec_create(sizeof(int), 1, false);
    assert(v);
    lvec_segment_t *seg = lvec_get_segment(v, 0);
    uint64_t expected_occupancy_map = 0;
    for(int i=0; i<64; i++) {
        uint8_t *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        assert(lvec_get_data_ptr(v, i) == ptr);
        assert(v->segment_count == 1);
        assert(v->element_count == i+1);
        expected_occupancy_map |= 1ULL << (uint64_t)i;
        assert(seg->occupancy_bitmap == expected_occupancy_map);
    }
    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_get_pointer_to_vacant_slot_empty_vec_2_segments(void) {
    TEST_STARTING;
    lvec_header_t  *v = lvec_create(sizeof(int), 2, false);
    assert(v);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);
    lvec_segment_t *seg1 = lvec_get_segment(v, 1);
    uint64_t expected_occupancy_map = 0;
    for(int i=0; i<64; i++) {
        uint8_t *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        assert(lvec_get_data_ptr(v, i) == ptr);
        assert(v->segment_count == 2);
        assert(v->element_count == i+1);
        expected_occupancy_map |= 1ULL << (uint64_t)i;
        assert(seg0->occupancy_bitmap == expected_occupancy_map);
        assert(seg1->occupancy_bitmap == 0);
    }
    expected_occupancy_map = 0;
    for(int i=64; i<128; i++) {
        uint8_t *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        assert(lvec_get_data_ptr(v, i) == ptr);
        assert(v->segment_count == 2);
        assert(v->element_count == i+1);
        expected_occupancy_map |= 1ULL << (uint64_t)i;
        assert(seg0->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF);
        assert(seg1->occupancy_bitmap == expected_occupancy_map);
    }
    assert(v->segment_count == 2);
    assert(v->element_count == 128);


    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment_expands_to_2(void) {
    TEST_STARTING;
    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);
    uint64_t expected_occupancy_map = 0;
    for(int i=0; i<64; i++) {
        uint8_t *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        assert(lvec_get_data_ptr(v, i) == ptr);
        assert(v->segment_count == 1);
        assert(v->element_count == i+1);
        expected_occupancy_map |= 1ULL << (uint64_t)i;
        assert(seg0->occupancy_bitmap == expected_occupancy_map);
    }
    assert(v->segment_count == 1);
    assert(v->element_count == 64);
    expected_occupancy_map = 0;
    for(int i=64; i<128; i++) {
        uint8_t *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        assert(v->segment_count == 2);
        assert(v->element_count == i+1);
        assert(lvec_get_data_ptr(v, i) == ptr);
        expected_occupancy_map |= 1ULL << (uint64_t)i;
        lvec_segment_t *seg1 = lvec_get_segment(v, 1);
        assert(seg0->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF);
        assert(seg1->occupancy_bitmap == expected_occupancy_map);
    }
    assert(v->segment_count == 2);
    assert(v->element_count == 128);

    lvec_free(v);
    TEST_PASSED;
}


void test_lvec_read_data_from_segment(void) {
    TEST_STARTING;

    srand(1232);

    int vals[128];
    for(int i=0; i<128; i++) vals[i] = rand() % 10000000;

    lvec_header_t  *v = lvec_create(sizeof(int), 2, false);
    assert(v);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);
    lvec_segment_t *seg1 = lvec_get_segment(v, 1);
    uint64_t expected_occupancy_map = 0;
    for(int i=0; i<128; i++) {
        int *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
        *ptr = vals[i];
    }
    assert(v->segment_count == 2);
    assert(v->element_count == 128);

    for(int i=0; i<128; i++) {
        int *ptr = (int*)lvec_get_data_ptr(v, i);
        assert(ptr);
        assert(*ptr == vals[i]);
    }

    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_vacate_slots_from_only_segment(void) {
    TEST_STARTING;

    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1);
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2);
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3);
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4);
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5);
    void *ptr6 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr6);

    assert(v->element_count == 6);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 3 |
        1ULL << 4 |
        1ULL << 5
    ));

    // delete slot 1, 3, 4
    assert(lvec_vacate_slot(v, 1));
    assert(lvec_vacate_slot(v, 3));
    assert(lvec_vacate_slot(v, 4));
    assert(v->element_count == 3);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 2 |
        1ULL << 5
    ));

    lvec_free(v);
    TEST_PASSED;
}


void test_lvec_vacate_slots_from_multiple_segments(void) {
    TEST_STARTING;

    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);

    for(int i=0; i<128; i++) {
        void *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
    }
    assert(v->element_count == 128);
    assert(v->segment_count == 2);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);
    lvec_segment_t *seg1 = lvec_get_segment(v, 1);
    assert(seg0->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF);
    assert(seg1->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF);

    assert(lvec_vacate_slot(v, 32)); // vacate slot in segment 0
    assert(lvec_vacate_slot(v, 96)); // vacate slot in segment 1
    assert(v->element_count == 126);
    assert(v->segment_count == 2);
    assert(seg0->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF ^ (1ULL << 32ULL));
    assert(seg1->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF ^ (1ULL << 32ULL));

    lvec_free(v);
    TEST_PASSED;
}


void test_lvec_get_ptr_to_single_segment_vacant_with_gaps(void) {
    TEST_STARTING;

    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);
    lvec_segment_t *seg0 = lvec_get_segment(v, 0);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1);
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2);
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3);
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4);
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5);
    void *ptr6 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr6);

    assert(v->element_count == 6);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 3 |
        1ULL << 4 |
        1ULL << 5
    ));

    // delete slot 1, 3, 4
    assert(lvec_vacate_slot(v, 1));
    assert(lvec_vacate_slot(v, 3));
    assert(lvec_vacate_slot(v, 4));
    assert(v->element_count == 3);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 2 |
        1ULL << 5
    ));

    void *new_ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(v->element_count == 4);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 5
    ));
    assert(new_ptr1 == lvec_get_data_ptr(v, 1));

    void *new_ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(v->element_count == 5);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 3 |
        1ULL << 5
    ));
    assert(new_ptr2 == lvec_get_data_ptr(v, 3));

    void *new_ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(v->element_count == 6);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 3 |
        1ULL << 4 |
        1ULL << 5
    ));
    assert(new_ptr3 == lvec_get_data_ptr(v, 4));

    void *new_ptr4 = lvec_get_pointer_to_vacant_slot(&v); // new slot at the end
    assert(v->element_count == 7);
    assert(seg0->occupancy_bitmap == (
        1ULL << 0 |
        1ULL << 1 |
        1ULL << 2 |
        1ULL << 3 |
        1ULL << 4 |
        1ULL << 5 |
        1ULL << 6
    ));
    assert(new_ptr4 == lvec_get_data_ptr(v, 6));

    lvec_free(v);
    TEST_PASSED;
}


void test_lvec_get_ptr_to_multisegment_vacant_with_gaps(void) {
    TEST_STARTING;

    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);

    for(int i=0; i<128; i++) {
        void *ptr = lvec_get_pointer_to_vacant_slot(&v);
        assert(ptr);
    }
    assert(v->element_count == 128);
    assert(v->segment_count == 2);

    // clear slots in segment 0 and 1
    assert(lvec_vacate_slot(v, 12));
    assert(lvec_vacate_slot(v, 44));
    assert(lvec_vacate_slot(v, 70));
    assert(lvec_vacate_slot(v, 101));
    assert(v->segment_count == 2);
    assert(v->element_count == 124);
    
    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1);
    assert(ptr1 = lvec_get_data_ptr(v, 12));
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2);
    assert(ptr2 = lvec_get_data_ptr(v, 44));
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3);
    assert(ptr3 = lvec_get_data_ptr(v, 70));
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4);
    assert(ptr4 = lvec_get_data_ptr(v, 101));
    assert(v->segment_count == 2);
    assert(v->element_count == 128);

    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5);
    assert(ptr5 = lvec_get_data_ptr(v, 128));
    assert(v->segment_count == 3);
    assert(v->element_count == 129);

    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_cannot_vacate_slot_that_is_out_of_bounds(void) {
    TEST_STARTING;
    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);

    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(v->segment_count == 1);
    assert(v->element_count == 5);

    assert(!lvec_vacate_slot(v, 66)); // no segment 2 exists.
    assert(v->segment_count == 1);
    assert(v->element_count == 5);

    lvec_free(v);
    TEST_PASSED;
}

void test_lvec_cannot_vacate_slot_that_is_unoccupied(void) {
    TEST_STARTING;
    lvec_header_t  *v = lvec_create(sizeof(int), 1, false);
    assert(v);

    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(lvec_get_pointer_to_vacant_slot(&v));
    assert(v->segment_count == 1);
    assert(v->element_count == 5);

    assert(!lvec_vacate_slot(v, 5)); // unoccupied slot
    assert(v->segment_count == 1);
    assert(v->element_count == 5);

    assert(lvec_vacate_slot(v, 4));
    assert(v->segment_count == 1);
    assert(v->element_count == 4);

    lvec_free(v);
    TEST_PASSED;
}

int main() {

    test_lvec_create();
    test_lvec_get_segment();
    test_lvec_get_slots_count();
    test_lvec_get_segment_ix_from_slot_ix();
    test_lvec_localize_slot_ix();
    test_lvec_get_data_ptr();

    test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment();
    test_lvec_get_pointer_to_vacant_slot_empty_vec_2_segments();
    test_lvec_get_pointer_to_vacant_slot_empty_vec_1_segment_expands_to_2();

    test_lvec_read_data_from_segment();

    test_lvec_vacate_slots_from_only_segment();
    test_lvec_vacate_slots_from_multiple_segments();

    test_lvec_get_ptr_to_single_segment_vacant_with_gaps();
    test_lvec_get_ptr_to_multisegment_vacant_with_gaps();

    test_lvec_cannot_vacate_slot_that_is_out_of_bounds();
    test_lvec_cannot_vacate_slot_that_is_unoccupied();

    return 0;
}
