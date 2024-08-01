
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lvec64.h"


#define TEST_STARTING printf("running test %s ", __func__)
#define TEST_PASSED printf("[ok]\n")

// TEST CRETE VECTOR
void test_lvec64_create() {
    TEST_STARTING;
    lvec64_t *v = lvec64_create(8, 16, 24);
    assert(v != NULL);
    assert(v->element_count == 0);
    assert(v->element_width == 8);
    assert(v->element_count_max == 16);
    assert(v->resize_quantity == 24);
    assert(v->occupancy_bitmap == 0x0000000000000000);
    lvec64_free(v);
    TEST_PASSED;
}

void test_newly_crated_lvec64_has_data_set_to_zeros() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = 16;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, 24);
    assert(v != NULL);
    assert(v->element_width == 8);
    assert(v->element_count_max == 16);
    for (
        uint32_t i = 0;
        i < element_width * initial_max_elements;
        i++
    ) assert(v->data[i] == 0);

    lvec64_free(v);
    TEST_PASSED;
}

void test_cannot_create_vector_with_initial_size_greater_than_LVEC64_MAX_ELEMENT_COUNT() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = LVEC64_MAX_ELEMENT_COUNT + 1;
    lvec64_t *v = lvec64_create(8, initial_max_elements, 24);
    assert(v == NULL);
    TEST_PASSED;
}

void test_can_create_vector_with_initial_size_equal_to_LVEC64_MAX_ELEMENT_COUNT() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = LVEC64_MAX_ELEMENT_COUNT;
    lvec64_t *v = lvec64_create(8, initial_max_elements, 24);
    assert(v != NULL);
    assert(v->element_count_max == LVEC64_MAX_ELEMENT_COUNT);
    TEST_PASSED;
}


// TEST ALLOCATE POINTERS
void test_pointers_can_be_allocated_to_an_empty_vector() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = 16;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, 24);
    assert(v != NULL);
    void *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + element_width * 0);
    assert(v->occupancy_bitmap == (1ULL << 0));
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + element_width * 1);
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1));
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + element_width * 2);
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1 | 1ULL << 2));
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + element_width * 3);
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1 | 1ULL << 2 | 1ULL << 3));

    lvec64_free(v);
    TEST_PASSED;
}

void test_pointers_can_be_allocated_to_a_vector_with_gaps(){
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = 16;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, 24);
    assert(v != NULL);
    assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1 | 1ULL << 2 | 1ULL << 3));

    assert(lvec64_vacate_slot(v, 2));
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1 /*| 1ULL << 2*/ | 1ULL << 3));
    void *ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + element_width * 2);
    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 1 | 1ULL << 2 | 1ULL << 3));

    lvec64_free(v);
    TEST_PASSED;
}

void test_the_vector_can_be_expanded_when_a_pointer_is_allocated_to_a_full_vector() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t initial_max_elements = 16;
    uint32_t resize_quantity = 24;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, resize_quantity);
    assert(v != NULL);
    for(int i = 0; i < 16; i++)
        assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);

    assert(
        v->occupancy_bitmap
        == (
              1ULL << 0 | 1ULL << 1 | 1ULL << 2 | 1ULL << 3
            | 1ULL << 4 | 1ULL << 5 | 1ULL << 6 | 1ULL << 7
            | 1ULL << 8 | 1ULL << 9 | 1ULL << 10 | 1ULL << 11
            | 1ULL << 12 | 1ULL << 13 | 1ULL << 14 | 1ULL << 15
        )
    );
    assert(v->element_count == 16);
    assert(v->element_count_max == 16);

    void *ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(
        v->occupancy_bitmap
        == (
              1ULL << 0 | 1ULL << 1 | 1ULL << 2 | 1ULL << 3
            | 1ULL << 4 | 1ULL << 5 | 1ULL << 6 | 1ULL << 7
            | 1ULL << 8 | 1ULL << 9 | 1ULL << 10 | 1ULL << 11
            | 1ULL << 12 | 1ULL << 13 | 1ULL << 14 | 1ULL << 15
            | 1ULL << 16
        )
    );
    assert(ptr == v->data + element_width * 16);
    assert(v->element_count == 17);
    assert(v->element_count_max == (initial_max_elements + resize_quantity));

    lvec64_free(v);
    TEST_PASSED;
}

void test_pointers_cannot_be_allocated_to_a_vector_with_that_is_at_capacity_and_can_be_expanded_but_not_the_full_resize_quantity() {
    TEST_STARTING;
    uint32_t element_width = 24;
    uint32_t initial_max_elements = 30;
    uint32_t resize_quantity = 30;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, resize_quantity);
    assert(v != NULL);
    uint64_t expected_bitmap = 0ULL;
    for(uint64_t i = 0; i < 30; i++) {
        assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    }
    assert(v->element_count == 30);
    assert(v->element_count_max == 30);
    for(uint64_t i = 0; i < 30; i++) {
        assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    }
    assert(v->element_count == 60);
    assert(v->element_count_max == 60);
    assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
    assert(v->element_count == 61);
    assert(v->element_count_max == 64);
    lvec64_free(v);
    TEST_PASSED;
}

void test_pointers_cannot_be_allocated_to_a_vector_with_that_is_at_capacity_and_cannot_be_expanded() {
    TEST_STARTING;
    uint32_t element_width = 24;
    uint32_t initial_max_elements = 8;
    uint32_t resize_quantity = 8;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, resize_quantity);
    assert(v != NULL);
    uint64_t expected_bitmap = 0ULL;
    for(uint64_t i = 0; i < LVEC64_MAX_ELEMENT_COUNT; i++) {
        assert(lvec64_get_pointer_to_vacant_slot(&v) != NULL);
        expected_bitmap |= (1ULL << (uint64_t) i);
        assert(v->occupancy_bitmap == expected_bitmap);
    }

    assert(v->element_count == LVEC64_MAX_ELEMENT_COUNT);
    assert(lvec64_get_pointer_to_vacant_slot(&v) == NULL);

    lvec64_free(v);
    TEST_PASSED;
}


// TEST VACATE SLOT

typedef struct elem_t {
    float a;
    float b;
} elem_t;

int main() {

    // Test create vector functionality
    test_lvec64_create();
    test_newly_crated_lvec64_has_data_set_to_zeros();
    test_cannot_create_vector_with_initial_size_greater_than_LVEC64_MAX_ELEMENT_COUNT();
    test_can_create_vector_with_initial_size_equal_to_LVEC64_MAX_ELEMENT_COUNT();

    // Test allocate pointer functionality
    test_pointers_can_be_allocated_to_an_empty_vector();
    test_pointers_can_be_allocated_to_a_vector_with_gaps();
    test_the_vector_can_be_expanded_when_a_pointer_is_allocated_to_a_full_vector();
    test_pointers_cannot_be_allocated_to_a_vector_with_that_is_at_capacity_and_can_be_expanded_but_not_the_full_resize_quantity();
    test_pointers_cannot_be_allocated_to_a_vector_with_that_is_at_capacity_and_cannot_be_expanded();

    // test vacate slot functionality

    // test macros

    return 0;
}
