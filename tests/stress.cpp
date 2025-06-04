#include <gtest/gtest.h>
#include <random>
#include "persistent_array.h"

template <typename T, size_t N>
struct SlowPersistentArray {
  std::array<T, N> array;

  const T& operator[](size_t i) const { return array[i]; }

  template <typename... Args>
  SlowPersistentArray update(size_t index, Args&&... args) const {
    auto new_array = array;
    new_array[index] = T{std::forward<Args>(args)...};
    return SlowPersistentArray{new_array};
  }
};

TEST(TestStress, SingleElement) {
  const int ITERATIONS = 1'000'000;

  std::mt19937 rnd{};

  std::vector<persistent_array<int, 1>> fast = {{0}};
  std::vector<SlowPersistentArray<int, 1>> slow = {{{0}}};

  for (int i = 0; i < ITERATIONS; ++i) {
    int index = rnd() % (i + 1);
    int new_val = rnd();
    fast.push_back(fast[index].update(0, new_val));
    slow.push_back(slow[index].update(0, new_val));
  }

  for (int i = 0; i < ITERATIONS + 1; ++i) {
    ASSERT_EQ(fast[i][0], slow[i][0]);
  }
}

TEST(TestStress, MultipleElements) {
  const int ITERATIONS = 200'000;
  const int N = 5;

  std::mt19937 rnd{};

  std::array<int, N> initial{};
  std::vector<persistent_array<int, N>> fast = {
      persistent_array<int, N>{initial.begin()}};
  std::vector<SlowPersistentArray<int, N>> slow = {{initial}};

  for (int i = 0; i < ITERATIONS; ++i) {
    int index = rnd() % (i + 1);
    int position = rnd() % N;
    int new_val = rnd();
    fast.push_back(fast[index].update(position, new_val));
    slow.push_back(slow[index].update(position, new_val));
  }

  for (int i = 0; i < ITERATIONS + 1; ++i) {
    for (int j = 0; j < N; ++j) {
      ASSERT_EQ(fast[i][j], slow[i][j]);
    }
  }
}
