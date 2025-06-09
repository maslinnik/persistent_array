#include <gtest/gtest.h>
#include "persistent_array.h"

// clang-format off
template <typename T>
using MyTypes = ::testing::Types<
    base::Initial<T>,
    base::MySharedPtr<T>,
    base::FourFold<T>,
    base::EightFold<T>
    >;
// clang-format on

#define PA_TEST_SUITE(name, tp)     \
  template <typename Base>          \
  struct name : ::testing::Test {}; \
  TYPED_TEST_SUITE(name, MyTypes<tp>)
