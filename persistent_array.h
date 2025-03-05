#include <memory>
#include <numeric>
#include "inplace_vector"

template <typename T, size_t N>
class persistent_array {
  struct IntermediateNode;
  struct DataNode;

  struct BaseNode {
    size_t size;
  };

  struct IntermediateNode : BaseNode {
    std::shared_ptr<BaseNode> left, right;

    IntermediateNode(std::shared_ptr<BaseNode> left,
                     std::shared_ptr<BaseNode> right)
        : BaseNode(left->size + right->size),
          left(std::move(left)),
          right(std::move(right)) {}
  };

  struct DataNode : BaseNode {
    T x;

    template <typename... Args>
    DataNode(Args&&... args) : BaseNode(1), x(std::forward<Args>(args)...) {}
  };

  std::shared_ptr<BaseNode> root;

  template <bool IsConst>
  class BaseIterator {
    static const size_t STACK_SIZE = N == 1 ? 2 : std::bit_width(N - 1) + 1;
    using StackType = std::inplace_vector<BaseNode*, STACK_SIZE>;

    StackType stack;

    friend class persistent_array;

    void go_to_kth(size_t k) {
      while (stack.back()->size > 1) {
        auto intermediate_node = static_cast<IntermediateNode*>(stack.back());
        if (intermediate_node->left->size > k) {
          stack.push_back(intermediate_node->left.get());
        } else {
          k -= intermediate_node->left->size;
          stack.push_back(intermediate_node->right.get());
        }
      }
    }

    BaseIterator(BaseNode* root, size_t index) : stack({root}) {
      if (index < N) {
        go_to_kth(index);
      } else {
        stack.push_back(nullptr);
      }
    }

    explicit BaseIterator(const StackType& stack) : stack(stack) {}

   public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<IsConst, const T, T>;
    using pointer = value_type*;
    using reference = value_type&;

    BaseIterator(const BaseIterator&) = default;
    BaseIterator& operator=(const BaseIterator&) = default;

    reference operator*() const {
      return static_cast<DataNode*>(stack.back())->x;
    }

    pointer operator->() const {
      return &static_cast<DataNode*>(stack.back())->x;
    }

    BaseIterator& operator+=(difference_type distance) {
      difference_type k = distance;
      if (!stack.back()) {
        stack.pop_back();
        k += N;
      }
      while (stack.size() > 1 && !(0 <= k && k < stack.back()->size)) {
        auto parent = static_cast<IntermediateNode*>(stack[stack.size() - 2]);
        if (stack.back() == parent->right.get()) {
          k += parent->left->size;
        }
        stack.pop_back();
      }
      if (0 <= k && k < stack.back()->size) {
        go_to_kth(k);
      } else {
        stack.push_back(nullptr);
      }
      return *this;
    }

    BaseIterator& operator-=(difference_type distance) {
      return operator+=(-distance);
    }

    BaseIterator operator+(difference_type i) const {
      BaseIterator result = *this;
      result += i;
      return result;
    }

    BaseIterator operator-(difference_type i) const {
      BaseIterator result = *this;
      result -= i;
      return result;
    }

    BaseIterator& operator++() { return operator+=(1); }

    BaseIterator operator++(int) {
      BaseIterator copy = *this;
      operator++();
      return copy;
    }

    BaseIterator& operator--() { return operator-=(1); }

    BaseIterator operator--(int) {
      BaseIterator copy = *this;
      operator--();
      return copy;
    }

    std::strong_ordering operator<=> (const BaseIterator& other) const {
      if (stack.back() == other.stack.back()) {
        return std::strong_ordering::equal;
      }
      size_t depth = std::min(stack.size(), other.stack.size()) - 1;
      while (stack[depth] != other.stack[depth]) {
        --depth;
      }
      return static_cast<IntermediateNode*>(stack[depth])->left.get() ==
                     stack[depth + 1]
                 ? std::strong_ordering::less
                 : std::strong_ordering::greater;
    }

    bool operator==(const BaseIterator& other) const {
      return stack.back() == other.stack.back();
    }

    operator BaseIterator<true>() { return BaseIterator<true>(stack); }
  };

  std::shared_ptr<BaseNode> build(size_t l, size_t r) {
    if (l + 1 == r) {
      return std::make_shared<DataNode>();
    } else {
      size_t m = std::midpoint(l, r);
      auto left = build(l, m);
      auto right = build(m, r);
      return std::make_shared<IntermediateNode>(std::move(left),
                                                std::move(right));
    }
  }

  template <std::input_iterator Iter>
  std::shared_ptr<BaseNode> build_from_iterator(size_t l, size_t r,
                                                Iter& iter) {
    if (l + 1 == r) {
      return std::make_shared<DataNode>(*iter++);
    } else {
      size_t m = std::midpoint(l, r);
      auto left = build_from_iterator(l, m, iter);
      auto right = build_from_iterator(m, r, iter);
      return std::make_shared<IntermediateNode>(std::move(left),
                                                std::move(right));
    }
  }

  template <typename... Args>
  std::shared_ptr<BaseNode> updated_node(BaseNode* curr, size_t i,
                                         Args&&... args) const {
    if (curr->size == 1) {
      return std::make_shared<DataNode>(std::forward<Args>(args)...);
    }
    auto intermediate_node = static_cast<IntermediateNode*>(curr);
    if (i < intermediate_node->left->size) {
      auto new_left = updated_node(intermediate_node->left.get(), i,
                                   std::forward<Args>(args)...);
      return std::make_shared<IntermediateNode>(std::move(new_left),
                                                intermediate_node->right);
    } else {
      auto new_right = updated_node(intermediate_node->right.get(),
                                    i - intermediate_node->left->size,
                                    std::forward<Args>(args)...);
      return std::make_shared<IntermediateNode>(intermediate_node->left,
                                                std::move(new_right));
    }
  }

  explicit persistent_array(std::shared_ptr<BaseNode> root)
      : root(std::move(root)) {}

 public:
  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return {root.get(), 0}; }

  iterator end() { return {root.get(), N}; }

  const_iterator begin() const { return cbegin(); }

  const_iterator end() const { return cend(); }

  const_iterator cbegin() const { return {root.get(), 0}; }

  const_iterator cend() const { return {root.get(), N}; }

  reverse_iterator rbegin() { return std::reverse_iterator(end()); }

  reverse_iterator rend() { return std::reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const { return crbegin(); }

  const_reverse_iterator rend() const { return crend(); }

  const_reverse_iterator crbegin() const {
    return std::reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::reverse_iterator(cbegin());
  }

  std::array<T, N> as_array() const requires(std::is_copy_constructible_v<T>) {
    alignas(alignof(T)) std::byte buffer[sizeof(T) * N];
    T* ptr = reinterpret_cast<T*>(buffer);
    for (const T& x : *this) {
      new (ptr++) T(x);
    }
    return std::to_array(std::move(reinterpret_cast<T(&)[N]>(buffer)));
  }

  std::array<T, N> to_array() {
    alignas(alignof(T)) std::byte buffer[sizeof(T) * N];
    T* ptr = reinterpret_cast<T*>(buffer);
    for (T& x : *this) {
      new (ptr++) T(std::move(x));
    }
    return std::to_array(std::move(reinterpret_cast<T(&)[N]>(buffer)));
  }

  explicit persistent_array() { root = build(0, N); }

  persistent_array(std::initializer_list<T> il) {
    auto iter = il.begin();
    root = build_from_iterator(0, N, iter);
  }

  template <std::input_iterator Iter>
  explicit persistent_array(Iter first) {
    root = build_from_iterator(0, N, first);
  }

  template <typename... Args>
  persistent_array update(size_t index, Args&&... args) const {
    return persistent_array(
        updated_node(root.get(), index, std::forward<Args>(args)...));
  }

  const T& operator[](size_t i) const { return *(begin() + i); }
};