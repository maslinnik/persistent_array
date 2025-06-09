#include <gtest/gtest.h>
#include "persistent_array.h"
#include "util.h"

PA_TEST_SUITE(TestCreate, int);

TYPED_TEST(TestCreate, Create) {
  persistent_array<int, TypeParam> pa = {179};
  std::array<int, 1> a = {179};
  ASSERT_TRUE(std::equal(a.begin(), a.end(), pa.begin()));
}

TYPED_TEST(TestCreate, CreatePowerOfTwo) {
  persistent_array<int, TypeParam> pa = {1, 2, 3, 4, 5, 6, 7, 8};
  std::array<int, 8> a = {1, 2, 3, 4, 5, 6, 7, 8};
  ASSERT_TRUE(std::equal(a.begin(), a.end(), pa.begin()));
}

TYPED_TEST(TestCreate, CreateNotPowerOfTwo) {
  persistent_array<int, TypeParam> pa = {1, 2, 3, 4, 5, 6, 7};
  std::array<int, 7> a = {1, 2, 3, 4, 5, 6, 7};
  ASSERT_TRUE(std::equal(a.begin(), a.end(), pa.begin()));
}

PA_TEST_SUITE(TestIndex, int);

TYPED_TEST(TestIndex, Traverse) {
  std::vector<int> v = {3, 1, 4, 1, 5, 9, 2};
  persistent_array<int, TypeParam> pa(v.begin(), v.end());
  for (int i = 0; i < 7; ++i) {
    ASSERT_EQ(v[i], pa[i]);
  }
}

PA_TEST_SUITE(TestUpdate, int);

TYPED_TEST(TestUpdate, SimpleUpdate) {
  persistent_array<int, TypeParam> pa = {1, 2, 3, 4, 5};
  auto new_pa = pa.update(4, -6);
  std::array<int, 5> a = {1, 2, 3, 4, -6};
  ASSERT_TRUE(std::equal(a.begin(), a.end(), new_pa.begin()));
}

TYPED_TEST(TestUpdate, Unchanged) {
  std::vector<persistent_array<int, TypeParam>> pa;
  pa.push_back({1, 2, 3});
  pa.push_back(pa[0].update(0, 8));
  pa.push_back(pa[1].update(2, 5));
  pa.push_back(pa[0].update(1, 7));
  std::vector<std::array<int, 3>> a = {
      {1, 2, 3}, {8, 2, 3}, {8, 2, 5}, {1, 7, 3}};
  for (int i = 0; i < 4; ++i) {
    ASSERT_TRUE(std::equal(a[i].begin(), a[i].end(), pa[i].begin()));
  }
}

PA_TEST_SUITE(TestIterators, int);

TYPED_TEST(TestIterators, TestAddition) {
  const int N = 10;

  std::array<int, N> a{};
  std::iota(a.begin(), a.end(), 0);
  persistent_array<int, TypeParam> pa{a.begin(), a.end()};

  ASSERT_EQ(pa.begin() + N, pa.end());

  for (int i = 0; i < N; ++i) {
    ASSERT_EQ(*(pa.begin() + i), i);
    ASSERT_EQ(*(pa.end() - (N - i)), i);
    for (int j = -i; i + j < N; ++j) {
      ASSERT_EQ((pa.begin() + i) + j, pa.begin() + (i + j));
    }
  }
}

TYPED_TEST(TestIterators, TestDifference) {
  const int N = 10;

  persistent_array<int, TypeParam> pa(N);

  ASSERT_EQ(pa.end() - pa.begin(), N);

  for (int i = 0; i < N; ++i) {
    ASSERT_EQ((pa.begin() + i) - pa.begin(), i);
    ASSERT_EQ(pa.end() - (pa.begin() + i), N - i);
    for (int j = 0; j < N; ++j) {
      ASSERT_EQ((pa.begin() + i) - (pa.begin() + j), i - j);
    }
  }
}

PA_TEST_SUITE(TestRequirements, int);

TYPED_TEST(TestRequirements, RandomAccessIterator) {
  using pa_t = persistent_array<int, TypeParam>;
  static_assert(std::random_access_iterator<typename pa_t::iterator>);
  static_assert(std::random_access_iterator<typename pa_t::const_iterator>);
  static_assert(std::random_access_iterator<typename pa_t::reverse_iterator>);
  static_assert(
      std::random_access_iterator<typename pa_t::const_reverse_iterator>);
}