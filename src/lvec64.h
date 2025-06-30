

#ifndef lv_LVEC64_H
#define lv_LVEC64_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LVEC64_MAX_ELEMENT_COUNT 64

#define lvec64_index_is_occupied(v, i) (v->occupancy_bitmap & (1ULL << i))

typedef struct {
    uint32_t element_width;
    uint32_t element_count;
    uint32_t element_count_max;
    uint32_t resize_quantity;
    uint64_t occupancy_bitmap;
    uint8_t  data[ ];
} lv_LVEC64;


lv_LVEC64* lvec64_create(
    uint32_t element_width,
    uint32_t initial_max_elements,
    uint32_t resize_quantity
) {
    if(initial_max_elements > LVEC64_MAX_ELEMENT_COUNT) return NULL;
    void *ptr = calloc(
        1,
        sizeof(lv_LVEC64) + (element_width * initial_max_elements)
    );
    if (ptr == NULL) return NULL;

    lv_LVEC64 *v = (lv_LVEC64 *) ptr;
    v->element_width = element_width;
    // v->element_count = 0; // calloc already sets this to 0
    v->resize_quantity = resize_quantity;
    v->element_count_max = initial_max_elements;
    // v->occupancy_bitmap = 0x0000000000000000; // calloc already sets this to 0
    return v;
}


#define lvec64_free(v) free(v)


void* lvec64_get_pointer_to_vacant_slot(lv_LVEC64 **v) {
    if((*v)->element_count >= LVEC64_MAX_ELEMENT_COUNT) return NULL;

    if((*v)->element_count < (*v)->element_count_max) {
        uint32_t new_index = __builtin_ffsll(~(*v)->occupancy_bitmap) - 1;
        void *ptr = (*v)->data + ((*v)->element_width * new_index);
        (*v)->occupancy_bitmap |= (1ULL << new_index);
        (*v)->element_count++;
        return ptr;
    }
    else {
        uint32_t slots_to_add =
            ((*v)->element_count + (*v)->resize_quantity) > LVEC64_MAX_ELEMENT_COUNT
            ? LVEC64_MAX_ELEMENT_COUNT - (*v)->element_count
            : (*v)->resize_quantity;

        void *expanded = realloc(
            *v,
            sizeof (lv_LVEC64)
            + ((*v)->element_count_max + slots_to_add) * (*v)->element_width
        );
        if(expanded == NULL) return NULL;
        (*v) = (lv_LVEC64*) expanded;
        memset(
            (*v)->data + ((*v)->element_width * (*v)->element_count_max),
            0,
            slots_to_add * (*v)->element_width
        );
        (*v)->element_count_max += slots_to_add;
        return lvec64_get_pointer_to_vacant_slot(v);
    }
}

bool lvec64_vacate_slot(lv_LVEC64 *v, uint32_t index) {
    if(
        (index >= LVEC64_MAX_ELEMENT_COUNT)
        || (index >= v->element_count_max)
        || !lvec64_index_is_occupied(v, index)
    ) return false;

    v->occupancy_bitmap &= ~(1ULL << index);
    v->element_count--;
    memset(v->data + (v->element_width * index), 0, v->element_width);
    return true;
}

#endif // LVEC_H