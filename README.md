# LooseVector
##  Vector Implementations in C


### lvec64_t

 - A resizable vector that holds elements of equal width. Data elements are stored in contiguous memory.
 - Max capacity is 64 elements.
   - The vector uses a `uint64_t` bitmap in order to find gaps.
   - Resizing is limited to the number of bits in `uint64_t`
 - `lvec64_get_pointer_to_vacant_slot` will reference bitmap in order to find the first vacant slot (slot with the lowest index).


Deleting items causes the vector to become fragmented. Adding new items fills in the gaps.

Iteration example:
```c

uint32_t found_elements = 0;
for(
    uint32_t i=0;
    i < v->element_count_max && found_elements < v->element_count;
    i++
){
    if(!lvec64_index_is_occupied(v, i)) continue;
    found_elements++;

}

```

When compiling with optimizations consider testing performance without `found_elements` counter.


<hr>
