# LooseVector
##  Vector Implementations in C


### lvec64_t

 - A resizable vector that holds elements of equal width.
 - Max capacity is 64 elements.
   - The vector uses a `uint64_t` bitmap in order to find gaps.
   - `lvec64_get_pointer_to_vacant_slot` will reference this bitmap in order to find the first vacant slot (lowest index).


<hr>

![](https://media1.tenor.com/m/c-S8cUwVVVEAAAAd/duck-dance.gif)