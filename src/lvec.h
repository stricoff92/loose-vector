
#ifndef LVEC_H
#define LVEC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct lvec_element_header_t {
    bool occupied;
} lvec_element_header_t;


typedef struct lvec_t {
    uint32_t element_width;
    uint32_t vector_capacity_element_count;
    uint32_t first_unoccupied_index;
    uint32_t vector_occupancy;
    uint8_t  data[ ];
} lvec_t;


lvec_t *lvec_create(
    uint32_t element_width,
    uint32_t vector_capacity_element_count,
    uint32_t resize_quantity
) {
    void *ptr = calloc(
        1,
        sizeof(lvec_t) + (element_width * vector_capacity_element_count)
    );
    if (ptr == NULL) return NULL;

    lvec_t *v = (lvec_t *) ptr;
    v->element_width = element_width;
    v->vector_capacity_element_count = vector_capacity_element_count;
    v->first_unoccupied_index = 0;
    v->vector_occupancy = 0;
    return v;
}

void* lvec_get_pointer_to_vacant_slot(lvec_t *v) {

}

void lvec_vacate_slot_at_index(lvec_t *v, uint32_t index) {

}

#endif
