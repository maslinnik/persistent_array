#include <memory>
#include <numeric>
#include "versions/initial_base.h"

template <typename T, typename Base = base::Initial<T>>
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

  size_t size() const { return base.size(); }

  size_t max_size() const { return Base::MAX_SIZE; }

  explicit persistent_array(size_t count) : base(Base::filled(count, T{})) {}

  explicit persistent_array(size_t count, const T& fill)
      : base(Base::filled(count, fill)) {}

  persistent_array(std::initializer_list<T> il)
      : base(Base::from_iter(il.begin(), il.end())) {}

  template <std::forward_iterator Iter>
  explicit persistent_array(Iter first, Iter last)
      : base(Base::from_iter(first, last)) {}

  template <typename... Args>
  persistent_array update(size_t index, Args&&... args) const {
    return persistent_array{base.update(index, std::forward<Args>(args)...)};
  }

  const T& operator[](size_t i) const { return *(begin() + i); }
};
