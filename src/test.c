
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lvec.h"


#define TEST_STARTING printf("running test %s ", __func__)
#define TEST_PASSED printf("[ok]\n")


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


int main() {

    test_lvec_can_be_created_with_initial_meta_data_properly_set();
    test_created_lvec_has_data_initialized_to_zeros();

    return 0;
}
