
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

void test_an_expanded_vector_has_its_new_memory_region_filled_with_zeros() {
    // When a vector is expanded, the new memory region should be filled with zeros.
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 2;
    uint32_t resize_quantity = 4;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;
    uint8_t *elem_width_ptr = (uint8_t *) &elem_width;
    uint8_t *initial_element_capacity_ptr = (uint8_t *) &initial_element_capacity;
    uint8_t *resize_quantity_ptr = (uint8_t *) &resize_quantity;
    uint8_t *initial_first_unoccupied_gap_index_ptr = (uint8_t *) &initial_first_unoccupied_gap_index;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    assert(v->vector_occupancy == 2);
    assert(v->vector_capacity_element_count == 2);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;

    uint32_t expected_vector_occupany = 2;
    uint8_t *expected_vector_occupany_ptr = (uint8_t *) &expected_vector_occupany;
    {
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
            expected_vector_occupany_ptr[0],
            expected_vector_occupany_ptr[1],
            expected_vector_occupany_ptr[2],
            expected_vector_occupany_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            1, 0, 0, 0,                                             // data[1]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->b
        };
        assert(sizeof (expected_data) == 44);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }

    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    assert(ptr3 == v->data + elem_width * 2);
    assert(v->vector_occupancy == 3);
    assert(v->vector_capacity_element_count == 6);
    {
        uint32_t element_capacity = 6;
        uint8_t *element_capacity_ptr = (uint8_t *) &element_capacity;
        uint32_t vector_occupancy = 3;
        uint8_t *vector_occupancy_ptr = (uint8_t *) &vector_occupancy;
        uint8_t expected_data[] = {
            elem_width_ptr[0],
            elem_width_ptr[1],
            elem_width_ptr[2],
            elem_width_ptr[3],
            element_capacity_ptr[0],
            element_capacity_ptr[1],
            element_capacity_ptr[2],
            element_capacity_ptr[3],
            resize_quantity_ptr[0],
            resize_quantity_ptr[1],
            resize_quantity_ptr[2],
            resize_quantity_ptr[3],
            initial_first_unoccupied_gap_index_ptr[0],
            initial_first_unoccupied_gap_index_ptr[1],
            initial_first_unoccupied_gap_index_ptr[2],
            initial_first_unoccupied_gap_index_ptr[3],
            vector_occupancy_ptr[0],
            vector_occupancy_ptr[1],
            vector_occupancy_ptr[2],
            vector_occupancy_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            1, 0, 0, 0,                                             // data[1]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->b
            1, 0, 0, 0,                                             // data[2]->header->occupied (newly provisioned)
            0, 0, 0, 0,                                             // data[2]->a                (newly provisioned)
            0, 0, 0, 0,                                             // data[2]->b                (newly provisioned)
            0, 0, 0, 0,                                             // data[3]->header->occupied
            0, 0, 0, 0,                                             // data[3]->a
            0, 0, 0, 0,                                             // data[3]->b
            0, 0, 0, 0,                                             // data[4]->header->occupied
            0, 0, 0, 0,                                             // data[4]->a
            0, 0, 0, 0,                                             // data[4]->b
            0, 0, 0, 0,                                             // data[4]->header->occupied
            0, 0, 0, 0,                                             // data[4]->a
            0, 0, 0, 0                                              // data[4]->b
        };
        assert(sizeof (expected_data) == 92);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }

    lvec_free(v);
    TEST_PASSED;
}


void test_procurement_of_new_pointer_on_a_gapped_vector_results_in_the_index_of_the_first_gap() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 8;
    uint32_t resize_quantity = 8;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);

    // slot 2 is vacated, now lets try to get a new pointer
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5 != NULL);
    assert(ptr5 == v->data + elem_width * 2);
    assert(((elem_t*) ptr5)->header.occupied);
    assert(v->vector_occupancy == 4);

    lvec_free(v);
    TEST_PASSED;
}

void test_get_new_pointer_to_first_gap_when_there_are_no_more_gaps_in_the_vector() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 8;
    uint32_t resize_quantity = 8;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);

    // slot 2 is vacated, now lets get a new pointer to the first gap
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5 != NULL);
    assert(ptr5 == v->data + elem_width * 2);
    assert(v->vector_occupancy == 4);

    // There are no more gaps in the vector, so the first_unoccupied_gap_index should be set to LVEC_NO_GAPS
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_free(v);
    TEST_PASSED;
}

void test_get_new_pointer_to_first_gap_when_there_are_more_gaps_in_the_vector() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 8;
    uint32_t resize_quantity = 8;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);
    lvec_vacate_slot_at_index(v, 1);
    assert(v->vector_occupancy == 2);
    assert(v->first_unoccupied_gap_index == 1);

    // slot 2 is vacated, now lets get a new pointer to the first gap (slot 1)
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5 != NULL);
    assert(ptr5 == v->data + elem_width * 1);
    // Vector now has 3 occupied slots, and the first gap is at index 2
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);

    // request a new pointer, this time we should get a pointer to slot 2
    void *ptr6 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr6 != NULL);
    assert(ptr6 == v->data + elem_width * 2);

    // Vector now has 4 occupied slots, and there are no more gaps
    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_free(v);
    TEST_PASSED;
}


void test_vacate_last_slot_on_a_gapless_vector() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 2;
    uint32_t resize_quantity = 4;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;
    uint8_t *elem_width_ptr = (uint8_t *) &elem_width;
    uint8_t *initial_element_capacity_ptr = (uint8_t *) &initial_element_capacity;
    uint8_t *resize_quantity_ptr = (uint8_t *) &resize_quantity;
    uint8_t *initial_first_unoccupied_gap_index_ptr = (uint8_t *) &initial_first_unoccupied_gap_index;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    assert(v->vector_occupancy == 2);
    assert(v->vector_capacity_element_count == 2);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;

    {
        uint32_t expected_vector_occupany = 2;
        uint8_t *expected_vector_occupany_ptr = (uint8_t *) &expected_vector_occupany;
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
            expected_vector_occupany_ptr[0],
            expected_vector_occupany_ptr[1],
            expected_vector_occupany_ptr[2],
            expected_vector_occupany_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            1, 0, 0, 0,                                             // data[1]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[1]->b
        };
        assert(sizeof (expected_data) == 44);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }

    lvec_vacate_slot_at_index(v, 1);
    assert(v->vector_occupancy == 1);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);
    {
        uint32_t expected_vector_occupany = 1;
        uint8_t *expected_vector_occupany_ptr = (uint8_t *) &expected_vector_occupany;
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
            expected_vector_occupany_ptr[0],
            expected_vector_occupany_ptr[1],
            expected_vector_occupany_ptr[2],
            expected_vector_occupany_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            0, 0, 0, 0,                                             // data[1]->header->occupied
            0, 0, 0, 0,                                             // data[1]->a
            0, 0, 0, 0                                              // data[1]->b
        };
        assert(sizeof (expected_data) == 44);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }

    lvec_free(v);
    TEST_PASSED;
}

void test_vacate_middle_slot_on_gapless_vector() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 5;
    uint32_t resize_quantity = 10;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 1);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 1);
    {
        uint32_t expected_vector_occupany = 3;
        int32_t expected_first_unoccupied_gap_index = 1;
        uint8_t *expected_vector_occupany_ptr = (uint8_t *) &expected_vector_occupany;
        uint8_t *elem_width_ptr = (uint8_t *) &elem_width;
        uint8_t *initial_element_capacity_ptr = (uint8_t *) &initial_element_capacity;
        uint8_t *resize_quantity_ptr = (uint8_t *) &resize_quantity;
        uint8_t *initial_first_unoccupied_gap_index_ptr = (uint8_t *) &expected_first_unoccupied_gap_index;
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
            expected_vector_occupany_ptr[0],
            expected_vector_occupany_ptr[1],
            expected_vector_occupany_ptr[2],
            expected_vector_occupany_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            0, 0, 0, 0,                                             // data[1]->header->occupied  (recently vacated)
            0, 0, 0, 0,                                             // data[1]->a
            0, 0, 0, 0,                                             // data[1]->b
            1, 0, 0, 0,                                             // data[2]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[2]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[2]->b
            1, 0, 0, 0,                                             // data[3]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[3]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[3]->b
            0, 0, 0, 0,                                             // data[4]->header->occupied  (never filled)
            0, 0, 0, 0,                                             // data[4]->a
            0, 0, 0, 0                                              // data[4]->b
        };
        assert(sizeof (expected_data) == 80);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }


    lvec_free(v);
    TEST_PASSED;
}

void test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_before_the_existing_gap() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 5;
    uint32_t resize_quantity = 10;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);


    // slot 2 is vacated, now lets empty slot 1 (which is before the first gap)
    lvec_vacate_slot_at_index(v, 1);
    assert(v->vector_occupancy == 2);
    assert(v->first_unoccupied_gap_index == 1);

    {
        uint32_t expected_vector_occupany = 2;
        int32_t expected_first_unoccupied_gap_index = 1;
        uint8_t *expected_vector_occupany_ptr = (uint8_t *) &expected_vector_occupany;
        uint8_t *elem_width_ptr = (uint8_t *) &elem_width;
        uint8_t *initial_element_capacity_ptr = (uint8_t *) &initial_element_capacity;
        uint8_t *resize_quantity_ptr = (uint8_t *) &resize_quantity;
        uint8_t *initial_first_unoccupied_gap_index_ptr = (uint8_t *) &expected_first_unoccupied_gap_index;
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
            expected_vector_occupany_ptr[0],
            expected_vector_occupany_ptr[1],
            expected_vector_occupany_ptr[2],
            expected_vector_occupany_ptr[3],
            1, 0, 0, 0,                                             // data[0]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[0]->b
            0, 0, 0, 0,                                             // data[1]->header->occupied  (recently vacated)
            0, 0, 0, 0,                                             // data[1]->a
            0, 0, 0, 0,                                             // data[1]->b
            0, 0, 0, 0,                                             // data[2]->header->occupied  (recently vacated)
            0, 0, 0, 0,                                             // data[2]->a
            0, 0, 0, 0,                                             // data[2]->b
            1, 0, 0, 0,                                             // data[3]->header->occupied
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[3]->a
            value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3], // data[3]->b
            0, 0, 0, 0,                                             // data[4]->header->occupied  (never filled)
            0, 0, 0, 0,                                             // data[4]->a
            0, 0, 0, 0                                              // data[4]->b
        };
        assert(sizeof (expected_data) == 80);
        assert(memcmp(v, expected_data, sizeof (expected_data)) == 0);
    }

    TEST_PASSED;
}

void test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_after_the_existing_gap_and_there_is_no_other_occupied_slot_after() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 8;
    uint32_t resize_quantity = 8;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;

    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);

    // slot 2 is vacated, now lets empty slot 3 (which is after the first gap)
    lvec_vacate_slot_at_index(v, 3);
    assert(v->vector_occupancy == 2);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_free(v);
    TEST_PASSED;
}

void test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_after_the_existing_gap_and_there_is_an_occupied_slot_after() {
    TEST_STARTING;
    float value = 123.45;
    uint8_t *value_ptr = (uint8_t*) &value;

    uint32_t elem_width = sizeof (elem_t);
    uint32_t initial_element_capacity = 8;
    uint32_t resize_quantity = 8;
    int32_t initial_first_unoccupied_gap_index = LVEC_NO_GAPS;

    lvec_t *v = lvec_create(elem_width, initial_element_capacity, resize_quantity);
    assert(v != NULL);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    void *ptr1 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr1 != NULL);
    ((elem_t*) ptr1)->a = value;
    ((elem_t*) ptr1)->b = value;
    void *ptr2 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr2 != NULL);
    ((elem_t*) ptr2)->a = value;
    ((elem_t*) ptr2)->b = value;
    void *ptr3 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr3 != NULL);
    ((elem_t*) ptr3)->a = value;
    ((elem_t*) ptr3)->b = value;
    void *ptr4 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr4 != NULL);
    ((elem_t*) ptr4)->a = value;
    ((elem_t*) ptr4)->b = value;
    void *ptr5 = lvec_get_pointer_to_vacant_slot(&v);
    assert(ptr5 != NULL);
    ((elem_t*) ptr5)->a = value;
    ((elem_t*) ptr5)->b = value;

    assert(v->vector_occupancy == 5);
    assert(v->first_unoccupied_gap_index == LVEC_NO_GAPS);

    lvec_vacate_slot_at_index(v, 2);
    assert(v->vector_occupancy == 4);
    assert(v->first_unoccupied_gap_index == 2);

    // slot 2 is vacated, now lets empty slot 3 (which is after the first gap and before the last element in the vector)
    lvec_vacate_slot_at_index(v, 3);
    assert(v->vector_occupancy == 3);
    assert(v->first_unoccupied_gap_index == 2);

    lvec_free(v);
    TEST_PASSED;
}

int main() {

    // sanity check
    assert(sizeof (elem_t) == 12);

    // test create vector
    test_lvec_can_be_created_with_initial_meta_data_properly_set();
    test_created_lvec_has_data_initialized_to_zeros();

    // test provision slots to tightly packed vector
    test_pointers_can_be_provisioned_to_a_gapless_vector_that_has_sufficient_capacity();
    test_a_vector_can_be_expanded_and_pointers_can_be_provisioned_if_the_vector_is_completely_filled();
    test_an_expanded_vector_has_its_new_memory_region_filled_with_zeros();
    test_procurement_of_new_pointer_on_a_gapped_vector_results_in_the_index_of_the_first_gap();
    test_get_new_pointer_to_first_gap_when_there_are_no_more_gaps_in_the_vector();
    test_get_new_pointer_to_first_gap_when_there_are_more_gaps_in_the_vector();

    // test vacate slots
    test_vacate_last_slot_on_a_gapless_vector();
    test_vacate_middle_slot_on_gapless_vector();
    test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_before_the_existing_gap();
    test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_after_the_existing_gap_and_there_is_no_other_occupied_slot_after();
    test_vacate_slot_in_vector_that_has_gaps_and_the_slot_were_vacating_is_after_the_existing_gap_and_there_is_an_occupied_slot_after();

    return 0;
}
