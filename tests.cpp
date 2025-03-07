#include <gtest/gtest.h>
#include "persistent_array.h"

TEST(TestCreate, CreateOne) {
  persistent_array<int, 1> pa = {179};
  std::array<int, 1> a = {179};
  ASSERT_EQ(a, pa.as_array());
}

TEST(TestCreate, CreatePowerOfTwo) {
  persistent_array<int, 8> pa = {1, 2, 3, 4, 5, 6, 7, 8};
  std::array<int, 8> a = {1, 2, 3, 4, 5, 6, 7, 8};
  ASSERT_EQ(a, pa.as_array());
}

TEST(TestCreate, CreateNotPowerOfTwo) {
  persistent_array<int, 7> pa = {1, 2, 3, 4, 5, 6, 7};
  std::array<int, 7> a = {1, 2, 3, 4, 5, 6, 7};
  ASSERT_EQ(a, pa.as_array());
}

TEST(TestIndex, Traverse) {
  std::vector<int> v = {3, 1, 4, 1, 5, 9, 2};
  persistent_array<int, 7> pa(v.begin());
  for (int i = 0; i < 7; ++i) {
    ASSERT_EQ(v[i], pa[i]);
  }
}

TEST(TestUpdate, SimpleUpdate) {
  persistent_array<int, 5> pa = {1, 2, 3, 4, 5};
  auto new_pa = pa.update(4, -6);
  std::array<int, 5> a = {1, 2, 3, 4, -6};
  ASSERT_EQ(a, new_pa.as_array());
}

TEST(TestUpdate, Unchanged) {
  std::vector<persistent_array<int, 3>> pa;
  pa.push_back({1, 2, 3});
  pa.push_back(pa[0].update(0, 8));
  pa.push_back(pa[1].update(2, 5));
  pa.push_back(pa[0].update(1, 7));
  std::vector<std::array<int, 3>> a = {
      {1, 2, 3}, {8, 2, 3}, {8, 2, 5}, {1, 7, 3}};
  for (int i = 0; i < 4; ++i) {
    ASSERT_EQ(a[i], pa[i].as_array());
  }
}

TEST(TestRequirements, RandomAccessIterator) {
  static_assert(std::random_access_iterator<
                typename persistent_array<int, 8>::iterator>);
  static_assert(std::random_access_iterator<
                typename persistent_array<int, 8>::const_iterator>);
  static_assert(std::random_access_iterator<
                typename persistent_array<int, 8>::reverse_iterator>);
  static_assert(std::random_access_iterator<
                typename persistent_array<int, 8>::const_reverse_iterator>);
}