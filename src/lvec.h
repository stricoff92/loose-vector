
#ifndef LVEC64_H
#define LVEC64_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LVEC_SEGMENT_SIZE 64


typedef uint64_t lvec64_occupancy_map_t;

typedef struct lvec_header_t {
    uint32_t element_width;
    uint32_t element_count;
    uint32_t segment_count;
} lvec_header_t; 

typedef struct lvec_segment_t {
    lvec64_occupancy_map_t occupancy_bitmap;
    uint8_t data[ ];
} lvec_segment_t;


#define lvec_get_segment(head, segment_ix) \
    (lvec_segment_t*) \
    (\
        ((uint8_t*)(head)) \
        + (sizeof (lvec_header_t) + (head->element_width * LVEC_SEGMENT_SIZE * segment_ix) + (sizeof (lvec64_occupancy_map_t) * segment_ix) )\
    )

void* lvec_create(uint32_t element_width) {
    lvec_header_t *ptr = (lvec_header_t*) calloc(1,
        sizeof(lvec_header_t)
        + sizeof (lvec64_occupancy_map_t)
        + (element_width * LVEC_SEGMENT_SIZE)
    );
    if(!ptr) return NULL;

    ptr->element_width = element_width;
    ptr->segment_count = 1;
    // ptr->element_count = 0; // calloc already sets this to 0
    return ptr;
}


#endif