
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lvec64.h"




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

void test_pointers_can_be_allocated_to_a_vector_that_is_at_capacity_and_can_be_expanded_but_not_the_full_resize_quantity() {
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

void test_pointers_cannot_be_allocated_to_a_vector_that_is_at_capacity_and_cannot_be_expanded() {
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

void test_newly_allocated_memory_regions_are_zeroed_out() {
    TEST_STARTING;
    float val = 12345.67;
    uint8_t *valp = (uint8_t*) &val;
    uint32_t element_width = 4;
    uint32_t initial_max_elements = 2;
    uint32_t resize_quantity = 2;
    lvec64_t *v = lvec64_create(element_width, initial_max_elements, resize_quantity);
    assert(v != NULL);
    float *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;

    {
        uint8_t* element_width_ptr = (uint8_t*) &element_width;
        uint8_t* initial_max_elements_ptr = (uint8_t*) &initial_max_elements;
        uint8_t* resize_quantity_ptr = (uint8_t*) &resize_quantity;
        uint32_t expected_element_count = 2;
        uint8_t *expected_element_count_ptr = (uint8_t*) &expected_element_count;
        uint64_t expected_occupancy_bitmap = (1 << 0UL | 1 << 1ULL);
        uint8_t *expected_occupancy_bitmap_ptr = (uint8_t*) &expected_occupancy_bitmap;
        uint8_t expected_memory[] = {
            element_width_ptr[0],
            element_width_ptr[1],
            element_width_ptr[2],
            element_width_ptr[3],
            expected_element_count_ptr[0],
            expected_element_count_ptr[1],
            expected_element_count_ptr[2],
            expected_element_count_ptr[3],
            initial_max_elements_ptr[0],
            initial_max_elements_ptr[1],
            initial_max_elements_ptr[2],
            initial_max_elements_ptr[3],
            resize_quantity_ptr[0],
            resize_quantity_ptr[1],
            resize_quantity_ptr[2],
            resize_quantity_ptr[3],
            expected_occupancy_bitmap_ptr[0],
            expected_occupancy_bitmap_ptr[1],
            expected_occupancy_bitmap_ptr[2],
            expected_occupancy_bitmap_ptr[3],
            expected_occupancy_bitmap_ptr[4],
            expected_occupancy_bitmap_ptr[5],
            expected_occupancy_bitmap_ptr[6],
            expected_occupancy_bitmap_ptr[7],
            valp[0], // data[0]
            valp[1], // data[0]
            valp[2], // data[0]
            valp[3], // data[0]
            valp[0], // data[1]
            valp[1], // data[1]
            valp[2], // data[1]
            valp[3], // data[1]
        };
        assert(memcmp(v, expected_memory, sizeof(expected_memory)) == 0);
    }

    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    {
        uint8_t* element_width_ptr = (uint8_t*) &element_width;
        uint32_t max_elements = 4;
        uint8_t* max_elements_ptr = (uint8_t*) &max_elements;
        uint8_t* resize_quantity_ptr = (uint8_t*) &resize_quantity;
        uint32_t expected_element_count = 3;
        uint8_t *expected_element_count_ptr = (uint8_t*) &expected_element_count;
        uint64_t expected_occupancy_bitmap = (1 << 0UL | 1 << 1ULL | 1 << 2ULL);
        uint8_t *expected_occupancy_bitmap_ptr = (uint8_t*) &expected_occupancy_bitmap;
        uint8_t expected_memory[] = {
            element_width_ptr[0],
            element_width_ptr[1],
            element_width_ptr[2],
            element_width_ptr[3],
            expected_element_count_ptr[0],
            expected_element_count_ptr[1],
            expected_element_count_ptr[2],
            expected_element_count_ptr[3],
            max_elements_ptr[0],
            max_elements_ptr[1],
            max_elements_ptr[2],
            max_elements_ptr[3],
            resize_quantity_ptr[0],
            resize_quantity_ptr[1],
            resize_quantity_ptr[2],
            resize_quantity_ptr[3],
            expected_occupancy_bitmap_ptr[0],
            expected_occupancy_bitmap_ptr[1],
            expected_occupancy_bitmap_ptr[2],
            expected_occupancy_bitmap_ptr[3],
            expected_occupancy_bitmap_ptr[4],
            expected_occupancy_bitmap_ptr[5],
            expected_occupancy_bitmap_ptr[6],
            expected_occupancy_bitmap_ptr[7],
            valp[0], // data[0]
            valp[1], // data[0]
            valp[2], // data[0]
            valp[3], // data[0]
            valp[0], // data[1]
            valp[1], // data[1]
            valp[2], // data[1]
            valp[3], // data[1]
            0, 0, 0, 0, // data[2] // newly minted slots are zeroed out
            0, 0, 0, 0  // data[3] //
        };
        assert(memcmp(v, expected_memory, sizeof(expected_memory)) == 0);
    }
    TEST_PASSED;
}


// TEST VACATE SLOT

void test_can_vacate_slot_that_is_filled() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t element_count_max = 16;
    uint32_t resize_quantity = 12;
    lvec64_t *v = lvec64_create(element_width, element_count_max, resize_quantity);
    assert(v != NULL);
    void *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);

    assert(lvec64_vacate_slot(v, 1));

    assert(v->occupancy_bitmap == (1ULL << 0 | 1ULL << 2));
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(ptr == v->data + 8 * 1);

    lvec64_free(v);
    TEST_PASSED;
}


void test_cannot_vacate_slot_that_is_outside_range_of_vector() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t element_count_max = 16;
    uint32_t resize_quantity = 12;
    lvec64_t *v = lvec64_create(element_width, element_count_max, resize_quantity);
    assert(v != NULL);

    uint32_t control_hash = hash_lvec64(v);
    assert(!lvec64_vacate_slot(v, element_count_max + 1));
    assert(!lvec64_vacate_slot(v, LVEC64_MAX_ELEMENT_COUNT));
    uint32_t test_hash = hash_lvec64(v);
    assert(control_hash == test_hash);

    lvec64_free(v);
    TEST_PASSED;
}


void test_vacating_slot_that_is_in_range_but_unoccupied_has_no_effect() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t element_count_max = 16;
    uint32_t resize_quantity = 12;
    lvec64_t *v = lvec64_create(element_width, element_count_max, resize_quantity);
    assert(v != NULL);
    float val = 12345.67;
    float *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;

    uint32_t control_hash = hash_lvec64(v);
    assert(!lvec64_vacate_slot(v, element_count_max - 1));
    uint32_t test_hash = hash_lvec64(v);
    assert(control_hash == test_hash);

    lvec64_free(v);
    TEST_PASSED;
}


void test_vacating_slot_zeros_out_memory_and_doesnt_change_adjacent_slots() {
    TEST_STARTING;
    uint32_t element_width = 4;
    uint32_t element_count_max = 16;
    uint32_t resize_quantity = 12;
    lvec64_t *v = lvec64_create(element_width, element_count_max, resize_quantity);
    assert(v != NULL);
    float val = 12345.67;
    float *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    *ptr = val;

    {
        uint8_t *element_width_ptr = (uint8_t*) &element_width;
        uint32_t max_elements = 16;
        uint8_t *max_elements_ptr = (uint8_t*) &max_elements;
        uint8_t *resize_quantity_ptr = (uint8_t*) &resize_quantity;
        uint32_t expected_element_count = 5;
        uint8_t *expected_element_count_ptr = (uint8_t*) &expected_element_count;
        uint64_t expected_occupancy_bitmap = (1 << 0UL | 1 << 1ULL | 1 << 2ULL | 1 << 3ULL | 1 << 4ULL);
        uint8_t *expected_occupancy_bitmap_ptr = (uint8_t*) &expected_occupancy_bitmap;
        uint8_t* valp = (uint8_t*) &val;
        uint8_t expected_memory[] = {
            element_width_ptr[0],
            element_width_ptr[1],
            element_width_ptr[2],
            element_width_ptr[3],
            expected_element_count_ptr[0],
            expected_element_count_ptr[1],
            expected_element_count_ptr[2],
            expected_element_count_ptr[3],
            max_elements_ptr[0],
            max_elements_ptr[1],
            max_elements_ptr[2],
            max_elements_ptr[3],
            resize_quantity_ptr[0],
            resize_quantity_ptr[1],
            resize_quantity_ptr[2],
            resize_quantity_ptr[3],
            expected_occupancy_bitmap_ptr[0],
            expected_occupancy_bitmap_ptr[1],
            expected_occupancy_bitmap_ptr[2],
            expected_occupancy_bitmap_ptr[3],
            expected_occupancy_bitmap_ptr[4],
            expected_occupancy_bitmap_ptr[5],
            expected_occupancy_bitmap_ptr[6],
            expected_occupancy_bitmap_ptr[7],
            valp[0], // data[0]
            valp[1], // data[0]
            valp[2], // data[0]
            valp[3], // data[0]
            valp[0], // data[1]
            valp[1], // data[1]
            valp[2], // data[1]
            valp[3], // data[1]
            valp[0], // data[2]
            valp[1], // data[2]
            valp[2], // data[2]
            valp[3], // data[2]
            valp[0], // data[3]
            valp[1], // data[3]
            valp[2], // data[3]
            valp[3], // data[3]
            valp[0], // data[4]
            valp[1], // data[4]
            valp[2], // data[4]
            valp[3]  // data[4]
        };
        assert(memcmp(v, expected_memory, sizeof(expected_memory)) == 0);
    }

    assert(lvec64_vacate_slot(v, 2));
    {
        uint8_t *element_width_ptr = (uint8_t*) &element_width;
        uint32_t max_elements = 16;
        uint8_t *max_elements_ptr = (uint8_t*) &max_elements;
        uint8_t *resize_quantity_ptr = (uint8_t*) &resize_quantity;
        uint32_t expected_element_count = 4;
        uint8_t *expected_element_count_ptr = (uint8_t*) &expected_element_count;
        uint64_t expected_occupancy_bitmap = (1 << 0UL | 1 << 1ULL /*| 1 << 2ULL */ | 1 << 3ULL | 1 << 4ULL);
        uint8_t *expected_occupancy_bitmap_ptr = (uint8_t*) &expected_occupancy_bitmap;
        uint8_t* valp = (uint8_t*) &val;
        uint8_t expected_memory[] = {
            element_width_ptr[0],
            element_width_ptr[1],
            element_width_ptr[2],
            element_width_ptr[3],
            expected_element_count_ptr[0],
            expected_element_count_ptr[1],
            expected_element_count_ptr[2],
            expected_element_count_ptr[3],
            max_elements_ptr[0],
            max_elements_ptr[1],
            max_elements_ptr[2],
            max_elements_ptr[3],
            resize_quantity_ptr[0],
            resize_quantity_ptr[1],
            resize_quantity_ptr[2],
            resize_quantity_ptr[3],
            expected_occupancy_bitmap_ptr[0],
            expected_occupancy_bitmap_ptr[1],
            expected_occupancy_bitmap_ptr[2],
            expected_occupancy_bitmap_ptr[3],
            expected_occupancy_bitmap_ptr[4],
            expected_occupancy_bitmap_ptr[5],
            expected_occupancy_bitmap_ptr[6],
            expected_occupancy_bitmap_ptr[7],
            valp[0], // data[0]
            valp[1], // data[0]
            valp[2], // data[0]
            valp[3], // data[0]
            valp[0], // data[1]
            valp[1], // data[1]
            valp[2], // data[1]
            valp[3], // data[1]
            0, 0, 0, 0,
            // valp[0], // data[2]
            // valp[1], // data[2]
            // valp[2], // data[2]
            // valp[3], // data[2]
            valp[0], // data[3]
            valp[1], // data[3]
            valp[2], // data[3]
            valp[3], // data[3]
            valp[0], // data[4]
            valp[1], // data[4]
            valp[2], // data[4]
            valp[3]  // data[4]
        };
        assert(memcmp(v, expected_memory, sizeof(expected_memory)) == 0);
    }

    lvec64_free(v);
    TEST_PASSED;
}

// Test Macros

void test_lvec64_index_is_occupied_macro() {
    TEST_STARTING;
    uint32_t element_width = 8;
    uint32_t element_count_max = 16;
    uint32_t resize_quantity = 12;
    lvec64_t *v = lvec64_create(element_width, element_count_max, resize_quantity);
    assert(v != NULL);
    void *ptr;
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    ptr = lvec64_get_pointer_to_vacant_slot(&v);
    assert(ptr != NULL);
    assert(lvec64_index_is_occupied(v, 0));
    assert(lvec64_index_is_occupied(v, 1));
    assert(lvec64_index_is_occupied(v, 2));
    assert(!lvec64_index_is_occupied(v, 3));

    assert(lvec64_vacate_slot(v, 1));
    assert(lvec64_index_is_occupied(v, 0));
    assert(!lvec64_index_is_occupied(v, 1));
    assert(lvec64_index_is_occupied(v, 2));
    assert(!lvec64_index_is_occupied(v, 3));

    lvec64_free(v);
    TEST_PASSED;
}


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
    test_pointers_can_be_allocated_to_a_vector_that_is_at_capacity_and_can_be_expanded_but_not_the_full_resize_quantity();
    test_pointers_cannot_be_allocated_to_a_vector_that_is_at_capacity_and_cannot_be_expanded();
    test_newly_allocated_memory_regions_are_zeroed_out();

    // test vacate slot functionality
    test_can_vacate_slot_that_is_filled();
    test_cannot_vacate_slot_that_is_outside_range_of_vector();
    test_vacating_slot_that_is_in_range_but_unoccupied_has_no_effect();
    test_vacating_slot_zeros_out_memory_and_doesnt_change_adjacent_slots();

    // test macros
    test_lvec64_index_is_occupied_macro();

    return 0;
}
