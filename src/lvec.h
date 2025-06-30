
#ifndef lv_LVEC_H
#define lv_LVEC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LVEC_SEGMENT_SIZE 64


typedef uint64_t lv_LVEC64_Occupancy_Bitmap;

typedef struct {
    uint32_t element_width;
    uint32_t element_count;
    uint32_t segment_count;
    bool     hard_delete_slots;
} lv_LVEC_Header;

typedef struct {
    lv_LVEC64_Occupancy_Bitmap occupancy_bitmap;
    uint8_t data[ ];
} lv_LVEC_Segment;


void* lvec_create(uint32_t element_width, uint32_t initial_segment_count, bool hard_delete_slots) {
    lv_LVEC_Header *ptr = (lv_LVEC_Header*) calloc(1,
        sizeof(lv_LVEC_Header)
        + (
            sizeof (lv_LVEC64_Occupancy_Bitmap)
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
    ((lv_LVEC_Segment*) \
    ( \
        ((uint8_t*)(head)) \
        + ( \
            (sizeof (lv_LVEC_Header)) \
            + ((head)->element_width * LVEC_SEGMENT_SIZE * (segment_ix)) \
            + (sizeof (lv_LVEC64_Occupancy_Bitmap) * (segment_ix)) \
        )\
    ))
#define lvec_get_slots_count(v) ((v)->segment_count * LVEC_SEGMENT_SIZE)
#define lvec_get_segment_ix_from_slot_ix(slot_ix) ((slot_ix) / LVEC_SEGMENT_SIZE)
#define lvec_localize_slot_ix(slot_ix) ((slot_ix) % LVEC_SEGMENT_SIZE)
#define lvec_local_slot_ix_is_occupied(seg, local_slot_ix) ((seg)->occupancy_bitmap & (1ULL << (local_slot_ix)))
#define segment_is_full(s) ((s)->occupancy_bitmap == 0xFFFFFFFFFFFFFFFF)
#define lvec_get_data_ptr(head, slot_ix) (\
    (lvec_get_segment((head), lvec_get_segment_ix_from_slot_ix(slot_ix))) \
    ->data + (head)->element_width * lvec_localize_slot_ix(slot_ix) \
)
#define lvec_slot_ix_is_occupied(head, slot_ix) (\
    lvec_local_slot_ix_is_occupied( \
        lvec_get_segment((head), lvec_get_segment_ix_from_slot_ix((slot_ix))), \
        lvec_localize_slot_ix((slot_ix)) \
    ) \
)

void* lvec_get_pointer_to_vacant_slot(lv_LVEC_Header **v) {
    if((*v)->element_count >= lvec_get_slots_count(*v)){
        uint32_t new_segment_count = (*v)->segment_count + 1;
        uint32_t new_size =
            sizeof(lv_LVEC_Header)
            + new_segment_count * (sizeof(lv_LVEC64_Occupancy_Bitmap) + (*v)->element_width * LVEC_SEGMENT_SIZE);
        void *expanded = realloc(*v, new_size);
        if(expanded == NULL) return NULL;
        *v = (lv_LVEC_Header*) expanded;
        memset(
            lvec_get_segment(*v, new_segment_count - 1),
            0,
            (sizeof(lv_LVEC64_Occupancy_Bitmap) + (*v)->element_width * LVEC_SEGMENT_SIZE)
        );
        (*v)->segment_count = new_segment_count;
    }
    for(uint32_t seg_ix = 0; seg_ix < (*v)->segment_count; seg_ix++) {
        lv_LVEC_Segment *seg = lvec_get_segment(*v, seg_ix);
        if(segment_is_full(seg)) continue;
        uint32_t new_localized_index = __builtin_ffsll(~seg->occupancy_bitmap) - 1;
        seg->occupancy_bitmap |= (1ULL << new_localized_index);
        (*v)->element_count++;
        return seg->data + (*v)->element_width * new_localized_index;
    }
    return NULL;
}


bool lvec_vacate_slot(lv_LVEC_Header *v, uint32_t slot_ix) {
    if(slot_ix >= lvec_get_slots_count(v))
        return false;
    lv_LVEC_Segment *seg = lvec_get_segment(v, lvec_get_segment_ix_from_slot_ix(slot_ix));
    uint32_t localized_slot_ix = lvec_localize_slot_ix(slot_ix);
    if(!lvec_local_slot_ix_is_occupied(seg, localized_slot_ix))
        return false;

    seg->occupancy_bitmap &= ~(1ULL << localized_slot_ix);
    v->element_count--;

    if(v->hard_delete_slots)
        memset(seg->data + v->element_width * localized_slot_ix, 0, v->element_width);
    return true;
}

// Iterator

typedef struct lv_LVEC_Iter {
    lv_LVEC_Header *head;
    lv_LVEC_Segment *seg;
    uint32_t seg_ix;
    uint8_t *element;
    uint8_t local_slot_ix_cursor;
} lv_LVEC_Iter;


lv_LVEC_Iter lvec_create_iter(lv_LVEC_Header *head) {
    lv_LVEC_Iter iter;
    iter.head = head;
    iter.seg = (lv_LVEC_Segment*) (((uint8_t*) head) + sizeof (lv_LVEC_Header));
    iter.seg_ix = 0;
    iter.element = ((uint8_t*)iter.seg) + sizeof (lv_LVEC64_Occupancy_Bitmap);
    iter.local_slot_ix_cursor = 0;
    return iter;
}

bool lvec_offset_iter_to_next_segment(lv_LVEC_Iter *iter) {
    if(iter->seg_ix == (iter->head->segment_count - 1))
        return false;

    iter->local_slot_ix_cursor = 0;
    iter->seg = (lv_LVEC_Segment*) (
        ((uint8_t*) iter->seg)
            + sizeof(lv_LVEC64_Occupancy_Bitmap)
            + iter->head->element_width * LVEC_SEGMENT_SIZE
    );
    iter->element = ((uint8_t*)iter->seg) + sizeof (lv_LVEC64_Occupancy_Bitmap);
    iter->seg_ix++;
    return true;
}

#endif