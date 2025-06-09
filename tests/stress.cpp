#include <gtest/gtest.h>
#include <random>
#include "persistent_array.h"
#include "util.h"

template <typename T>
struct SlowPersistentArray {
  std::vector<T> array;

  const T& operator[](size_t i) const { return array[i]; }

  template <typename... Args>
  SlowPersistentArray update(size_t index, Args&&... args) const {
    auto new_array = array;
    new_array[index] = T{std::forward<Args>(args)...};
    return SlowPersistentArray{new_array};
  }
};

PA_TEST_SUITE(TestStress, int);

TYPED_TEST(TestStress, Updates) {
  const int MAX_ITERS = 1'000'000;
  std::mt19937 rnd{};

  for (int n : {1, 2, 3, 4, 5, 50, 200, 1000}) {
    const int iters = MAX_ITERS / n;

    std::vector<int> initial(n, 0);
    std::vector<persistent_array<int>> fast;
    fast.emplace_back(initial.begin(), initial.end());
    std::vector<SlowPersistentArray<int>> slow;
    slow.emplace_back(initial);

    for (int i = 0; i < iters; ++i) {
      int index = rnd() % (i + 1);
      int new_val = rnd();
      fast.push_back(fast[index].update(0, new_val));
      slow.push_back(slow[index].update(0, new_val));
    }

    for (int i = 0; i < iters + 1; ++i) {
      ASSERT_EQ(fast[i][0], slow[i][0]);
    }
  }
}