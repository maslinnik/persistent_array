# persistent_array

_STL-compliant memory-efficient fully persistent array_

- implemented using persistent segment tree
- linear-time traversal
- random-access iterators
- low memory overhead for large types
- satisfies the requirements of [_Container_](https://en.cppreference.com/w/cpp/named_req/Container)

## Usage

```c++
#include "persistent_array.h"

/* ... */

persistent_array<int> ver_0 = {3, 1, 4, 1, 6};
auto ver_1 = ver_0.update(4, 5);
assert(ver_0[4] == 6);
assert(ver_1[4] == 5);
```