
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


lvec_t* lvec_create(
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

void lvec_free(lvec_t *v) {
    free(v);
}

void* lvec_get_pointer_to_vacant_slot(lvec_t **v) {
    if((*v)->vector_occupancy < (*v)->vector_capacity_element_count) {
        void *ptr;
        if((*v)->first_unoccupied_gap_index == LVEC_NO_GAPS) {
            ptr = (*v)->data + ((*v)->element_width * (*v)->vector_occupancy);
            ((lvec_element_header_t*) ptr)->occupied = true;
            (*v)->vector_occupancy++;
        } else {
            // return pointer to first unoccupied gap,
            // identify next unoccupied gap, or mark vec as having no gaps.

            // This is the pointer to the first unoccupied gap which will be returned.
            ptr = (*v)->data + ((*v)->element_width * (*v)->first_unoccupied_gap_index);
            (*v)->vector_occupancy++;

            // mark the newly occupied slot as occupied
            ((lvec_element_header_t*) ptr)->occupied = true;
            uint8_t *data = (uint8_t*) ptr;
            uint32_t element_ix = (*v)->first_unoccupied_gap_index;

            // find the next unoccupied gap, or verify that there are no more gaps.
            element_ix++;
            data += (*v)->element_width;
            bool found_gap = false;
            while(element_ix < (*v)->vector_occupancy) {
                if(((lvec_element_header_t*) data)->occupied == false) {
                    (*v)->first_unoccupied_gap_index = element_ix;
                    found_gap = true;
                    break;
                }
                element_ix++;
                data += (*v)->element_width;
            }
            if(!found_gap) (*v)->first_unoccupied_gap_index = LVEC_NO_GAPS;
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
    if(!v->vector_occupancy) return;
    if(index >= v->vector_capacity_element_count) return; // Guard against out of bounds access

    memset(
        v->data + (index * v->element_width),
        0,
        v->element_width
    );
    if(index < (v->vector_occupancy - 1)) {
        if(((int32_t) index) < v->first_unoccupied_gap_index) {
            v->first_unoccupied_gap_index = (int32_t) index;
        }
    }
    v->vector_occupancy--;
}



#endif
