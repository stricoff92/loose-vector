
#ifndef LVEC_H
#define LVEC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define LVEC_NO_GAPS -1

typedef struct lvec_element_header_t {
    bool occupied;
} lvec_element_header_t;


typedef struct lvec_t {
    uint32_t element_width;
    uint32_t vector_capacity_element_count;
    uint32_t resize_quantity;
    int32_t  first_unoccupied_gap_index;
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
    v->resize_quantity = resize_quantity;
    v->first_unoccupied_gap_index = LVEC_NO_GAPS;
    v->vector_occupancy = 0;
    return v;
}

void* lvec_get_pointer_to_vacant_slot(lvec_t **v) {
    if((*v)->vector_occupancy < (*v)->vector_capacity_element_count) {
        void *ptr;
        if((*v)->first_unoccupied_gap_index == LVEC_NO_GAPS) {
            ptr = (*v)->data + ((*v)->element_width * (*v)->vector_occupancy);
            (*v)->vector_occupancy++;
        } else {
            ptr = (*v)->data + ((*v)->element_width * (*v)->first_unoccupied_gap_index);
            uint8_t *data_ix = (*v)->data + ((*v)->element_width * (*v)->first_unoccupied_gap_index);
            ((lvec_element_header_t*) data_ix)->occupied = false;

            data_ix += (*v)->element_width;
            uint32_t next_gap_index = LVEC_NO_GAPS;
            while(true) {

            }
            (*v)->first_unoccupied_gap_index = next_gap_index;
        }
        return ptr;
    } else {
        void *expanded = realloc(
            *v,
            sizeof (lvec_t)
            + ((*v)->vector_capacity_element_count + (*v)->resize_quantity) * (*v)->element_width
        );
        if(expanded == NULL) return NULL;
        (*v) = (lvec_t*) expanded;
        memset(
            (*v)->data + ((*v)->element_width * (*v)->vector_capacity_element_count),
            0,
            (*v)->resize_quantity * (*v)->element_width
        );
        (*v)->vector_capacity_element_count += (*v)->resize_quantity;
        return lvec_get_pointer_to_vacant_slot(v);
    }
}

void lvec_vacate_slot_at_index(lvec_t *v, uint32_t index) {

}

#endif
