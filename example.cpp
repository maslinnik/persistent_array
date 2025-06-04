#include <cassert>
#include "persistent_array.h"

int main() {
  persistent_array<int, 5> pa = {1, 2, 3, 4, 5};
  auto new_pa = pa.update(4, -6);
  std::array<int, 5> a = {1, 2, 3, 4, -6};
  assert(a == new_pa.as_array());
}