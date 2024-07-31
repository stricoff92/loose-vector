
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lvec.h"


#define TEST_STARTING printf("running test %s ", __func__)
#define TEST_PASSED printf("[ok]\n")


typedef struct elem_t {
    lvec_element_header_t header;
    float a;
    float b;
} elem_t;

void test_lvec_can_be_created_with_initial_meta_data_properly_set() {
    TEST_STARTING;
    lvec_t *v = lvec_create(4, 10, 12);
    assert(v != NULL);
    assert(v->element_width == 4);
    assert(v->vector_capacity_element_count == 10);
    assert(v->resize_quantity == 12);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);
    lvec_free(v);
    TEST_PASSED;
}

void test_created_lvec_has_data_initialized_to_zeros() {
    TEST_STARTING;
    uint32_t elem_width = 4;
    uint32_t initial_element_capacity = 10;
    uint32_t resize_quantity = 10;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;
    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);

    uint8_t *elem_width_ptr = (uint8_t *) &elem_width;
    uint8_t *initial_element_capacity_ptr = (uint8_t *) &initial_element_capacity;
    uint8_t *resize_quantity_ptr = (uint8_t *) &resize_quantity;
    uint8_t * initial_first_unoccupied_gap_index_ptr = (uint8_t *) &initial_first_unoccupied_gap_index;
    uint8_t expected_data[] = {
        elem_width_ptr[0],
        elem_width_ptr[1],
        elem_width_ptr[2],
        elem_width_ptr[3],
        initial_element_capacity_ptr[0],
        initial_element_capacity_ptr[1],
        initial_element_capacity_ptr[2],
        initial_element_capacity_ptr[3],
        resize_quantity_ptr[0],
        resize_quantity_ptr[1],
        resize_quantity_ptr[2],
        resize_quantity_ptr[3],
        initial_first_unoccupied_gap_index_ptr[0],
        initial_first_unoccupied_gap_index_ptr[1],
        initial_first_unoccupied_gap_index_ptr[2],
        initial_first_unoccupied_gap_index_ptr[3],
        0, 0, 0, 0, // vector occupancy
        0, 0, 0, 0, // data [0]
        0, 0, 0, 0, // data [1]
        0, 0, 0, 0, // data [2]
        0, 0, 0, 0, // data [3]
        0, 0, 0, 0, // data [4]
        0, 0, 0, 0, // data [5]
        0, 0, 0, 0, // data [6]
        0, 0, 0, 0, // data [7]
        0, 0, 0, 0, // data [8]
        0, 0, 0, 0  // data [9]
    };
    assert(sizeof (expected_data) == 60);
    assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    lvec_free(v);
    TEST_PASSED;
}

void test_pointers_can_be_provisioned_to_a_gapless_vector_that_has_sufficient_capacity() {
    TEST_STARTING;
    uint32_t elem_width = sizeof (elem_t);
    lvec_t *v = lvec_create(elem_width, 10, 12);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    assert(ptr1 == v->data + elem_width * 0);
    assert(((elem_t *) ptr1)->header.occupied);
    assert(v->vector_occupancy == 1);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    assert(ptr2 == v->data + elem_width * 1);
    assert(((elem_t *) ptr2)->header.occupied);
    assert(v->vector_occupancy == 2);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    assert(ptr3 == v->data + elem_width * 2);
    assert(((elem_t *) ptr3)->header.occupied);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    assert(v->vector_capacity_element_count == 10); // no resizing should have occurred

    lvec_free(v);
    TEST_PASSED;
}

void test_a_vector_can_be_expanded_and_pointers_can_be_provisioned_if_the_vector_is_completely_filled() {
    TEST_STARTING;
    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_capacity = 4;
    uint32_t resize_quantity = 2;
    lvec_t *v = lvec_create(elem_width, initial_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);

    // vector is now full and we expect it to be resized when we try to provision a new pointer
    assert(v->vector_occupancy == 4);
    assert(v->vector_capacity_element_count == 4); // no resizing should have occurred
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5 != NULL);
    assert(ptr5 == v->data + elem_width * 4);
    assert(((elem_t *) ptr5)->header.occupied);
    assert(v->vector_occupancy == 5);
    assert(v->vector_capacity_element_count == 6);

    lvec_free(v);
    TEST_PASSED;
}

int main() {

    test_lvec_can_be_created_with_initial_meta_data_properly_set();
    test_created_lvec_has_data_initialized_to_zeros();
    test_pointers_can_be_provisioned_to_a_gapless_vector_that_has_sufficient_capacity();
    test_a_vector_can_be_expanded_and_pointers_can_be_provisioned_if_the_vector_is_completely_filled();

    return 0;
}
