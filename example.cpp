#include <cassert>
#include "persistent_array.h"

int main() {
  persistent_array<int> ver_0 = {3, 1, 4, 1, 6};
  auto ver_1 = ver_0.update(4, 5);
  assert(ver_0[4] == 6);
  assert(ver_1[4] == 5);
}