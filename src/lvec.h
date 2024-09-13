
#ifndef LVEC_H
#define LVEC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LVEC_SEGMENT_SIZE 64


typedef uint64_t lvec64_occupancy_bitmap_t;

typedef struct lvec_header_t {
    uint32_t element_width;
    uint32_t element_count;
    uint32_t segment_count;
    bool     hard_delete_slots;
} lvec_header_t; 

typedef struct lvec_segment_t {
    lvec64_occupancy_bitmap_t occupancy_bitmap;
    uint8_t data[ ];
} lvec_segment_t;


void* lvec_create(uint32_t element_width, uint32_t initial_segment_count, bool hard_delete_slots) {
    lvec_header_t *ptr = (lvec_header_t*) calloc(1,
        sizeof(lvec_header_t)
        + (
            sizeof (lvec64_occupancy_bitmap_t)
            + (element_width * LVEC_SEGMENT_SIZE)
        ) * initial_segment_count
    );
    if(!ptr) return NULL;

    ptr->element_width = element_width;
    ptr->segment_count = initial_segment_count;
    ptr->hard_delete_slots = hard_delete_slots;
    // ptr->element_count = 0; // calloc already sets this to 0
    return ptr;
}


#define lvec_free(v) free(v)


// Calculate a pointer to a given segment index
#define lvec_get_segment(head, segment_ix) \
    ((lvec_segment_t*) \
    ( \
        ((uint8_t*)(head)) \
        + ( \
            (sizeof (lvec_header_t)) \
            + ((head)->element_width * LVEC_SEGMENT_SIZE * (segment_ix)) \
            + (sizeof (lvec64_occupancy_bitmap_t) * (segment_ix)) \
        )\
    ))
#define lvec_get_slots_count(v) ((v)->segment_count * LVEC_SEGMENT_SIZE)
#define lvec_get_segment_ix_from_slot_ix(slot_ix) ((slot_ix) / LVEC_SEGMENT_SIZE)
#define lvec_localize_slot_ix(slot_ix) ((slot_ix) % LVEC_SEGMENT_SIZE)
#define lvec_local_slot_ix_is_occupied(seg, slot_ix) ((seg)->occupancy_bitmap & (1ULL << (slot_ix)))
#define segment_is_full(s) ((s)->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF)
#define lvec_get_data_ptr(head, slot_ix) (\
    (lvec_get_segment((head), lvec_get_segment_ix_from_slot_ix(slot_ix))) \
    ->data + (head)->element_width * lvec_localize_slot_ix(slot_ix) \
)


void* lvec_get_pointer_to_vacant_slot(lvec_header_t **v) {
    if((*v)->element_count >= lvec_get_slots_count(*v)){
        uint32_t new_segment_count = (*v)->segment_count + 1;
        uint32_t new_size =
            sizeof(lvec_header_t)
            + new_segment_count * (sizeof(lvec64_occupancy_bitmap_t) + (*v)->element_width * LVEC_SEGMENT_SIZE);
        void *expanded = realloc(*v, new_size);
        if(expanded == NULL) return NULL;
        *v = expanded;
        memset(
            lvec_get_segment(*v, new_segment_count - 1),
            0,
            (sizeof(lvec64_occupancy_bitmap_t) + (*v)->element_width * LVEC_SEGMENT_SIZE)
        );
        (*v)->segment_count = new_segment_count;
    }
    for(uint32_t seg_ix = 0; seg_ix < (*v)->segment_count; seg_ix++) {
        lvec_segment_t *seg = lvec_get_segment(*v, seg_ix);
        if(segment_is_full(seg)) continue;
        uint32_t new_localized_index = __builtin_ffsll(~seg->occupancy_bitmap) - 1;
        seg->occupancy_bitmap |= (1ULL << new_localized_index);
        (*v)->element_count++;
        return seg->data + (*v)->element_width * new_localized_index;
    }
    return NULL;
}


bool lvec_vacate_slot(lvec_header_t *v, uint32_t slot_ix) {
    if(slot_ix >= lvec_get_slots_count(v))
        return false;
    lvec_segment_t *seg = lvec_get_segment(v, lvec_get_segment_ix_from_slot_ix(slot_ix));
    uint32_t localized_slot_ix = lvec_localize_slot_ix(slot_ix);
    if(!lvec_local_slot_ix_is_occupied(seg, localized_slot_ix))
        return false;

    seg->occupancy_bitmap &= ~(1ULL << localized_slot_ix);
    v->element_count--;

    if(v->hard_delete_slots) 
        memset(seg->data + v->element_width * localized_slot_ix, 0, v->element_width);
    return true;
}

#endif