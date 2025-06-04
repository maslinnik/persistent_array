#include <memory>
#include <numeric>
#include "versions/initial_base.h"

template <typename T, size_t N, typename Base = base::Initial<T, N>>
class persistent_array {
  Base base;

  explicit persistent_array(Base base) : base(std::move(base)) {}

 public:
  using iterator = typename Base::template BaseIterator<true>;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = reverse_iterator;

  iterator begin() const { return base.begin(); }

  iterator end() const { return base.end(); }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() const { return std::reverse_iterator(end()); }

  reverse_iterator rend() const { return std::reverse_iterator(begin()); }

  const_reverse_iterator crbegin() const { return rbegin(); }

  const_reverse_iterator crend() const { return rend(); }

  std::array<T, N> as_array() const {
    alignas(alignof(T)) std::byte buffer[sizeof(T) * N];
    T* ptr = reinterpret_cast<T*>(buffer);
    for (const T& x : *this) {
      new (ptr++) T(x);
    }
    return std::to_array(std::move(reinterpret_cast<T(&)[N]>(buffer)));
  }

  /*
  std::array<T, N> to_array() {
    alignas(alignof(T)) std::byte buffer[sizeof(T) * N];
    T* ptr = reinterpret_cast<T*>(buffer);
    for (T& x : *this) {
      new (ptr++) T(std::move(x));
    }
    return std::to_array(std::move(reinterpret_cast<T(&)[N]>(buffer)));
  }
   */

  explicit persistent_array() : base(Base::filled(T{})) {}

  explicit persistent_array(const T& fill) : base(Base::filled(fill)) {}

  persistent_array(std::initializer_list<T> il)
      : base(Base::from_iter(il.begin())) {}

  template <std::input_iterator Iter>
  explicit persistent_array(Iter first) : base(Base::from_iter(first)) {}

  template <typename... Args>
  persistent_array update(size_t index, Args&&... args) const {
    return persistent_array{base.update(index, std::forward<Args>(args)...)};
  }

  const T& operator[](size_t i) const { return *(begin() + i); }
};
