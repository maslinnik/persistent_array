#include <memory>
#include <numeric>

#include "../inplace_vector"

namespace base {

template <typename T, size_t N>
struct Initial {
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

    friend class Initial;

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

    BaseIterator() = default;
    BaseIterator(const BaseIterator&) = default;
    BaseIterator& operator=(const BaseIterator&) = default;

    reference operator*() const {
      return static_cast<DataNode*>(stack.back())->x;
    }

    pointer operator->() const {
      return &static_cast<DataNode*>(stack.back())->x;
    }

    reference operator[](difference_type n) const { return *operator+(n); }

    BaseIterator& operator+=(difference_type n) {
      difference_type k = n;
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

    BaseIterator& operator-=(difference_type n) { return operator+=(-n); }

    BaseIterator operator+(difference_type n) const {
      BaseIterator result = *this;
      result += n;
      return result;
    }

    BaseIterator operator-(difference_type n) const {
      BaseIterator result = *this;
      result -= n;
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

    difference_type operator-(const BaseIterator& other) const {
      size_t lca_depth = std::min(stack.size(), other.stack.size()) - 1;
      while (stack[lca_depth] != other.stack[lca_depth]) {
        --lca_depth;
      }
      auto lca_index = [&](const StackType& stack) {
        if (!stack.back()) {
          return N;
        }
        size_t result = 0;
        for (size_t i = lca_depth; i + 1 < stack.size(); ++i) {
          auto intermediate_node = static_cast<IntermediateNode*>(stack[i]);
          if (intermediate_node->right.get() == stack[i + 1]) {
            result += intermediate_node->left->size;
          }
        }
        return result;
      };
      return lca_index(stack) - lca_index(other.stack);
    }

    std::strong_ordering operator<=>(const BaseIterator& other) const {
      return operator-(other) <=> 0;
    }

    bool operator==(const BaseIterator& other) const {
      return stack.back() == other.stack.back();
    }

    friend BaseIterator operator+(difference_type i, const BaseIterator& iter) {
      return iter + i;
    }

    explicit operator BaseIterator<true>() { return BaseIterator<true>(stack); }
  };

  explicit Initial(std::shared_ptr<BaseNode> root) : root(std::move(root)) {}

  BaseIterator<true> begin() const { return {root.get(), 0}; }

  BaseIterator<true> end() const { return {root.get(), N}; }

  BaseIterator<false> mutable_begin() { return {root.get(), 0}; }

  BaseIterator<false> mutable_end() { return {root.get(), N}; }

  template <std::input_iterator Iter>
  static std::shared_ptr<BaseNode> build_from_iter(size_t l, size_t r,
                                                   Iter& iter) {
    if (l + 1 == r) {
      return std::make_shared<DataNode>(*iter++);
    } else {
      size_t m = std::midpoint(l, r);
      auto left = build_from_iter(l, m, iter);
      auto right = build_from_iter(m, r, iter);
      return std::make_shared<IntermediateNode>(std::move(left),
                                                std::move(right));
    }
  }

  static std::shared_ptr<BaseNode> build_filled(size_t l, size_t r,
                                                const T& fill) {
    if (l + 1 == r) {
      return std::make_shared<DataNode>(fill);
    } else {
      size_t m = std::midpoint(l, r);
      auto left = build_from_iterator(l, m, fill);
      auto right = build_from_iterator(m, r, fill);
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

  static Initial filled(const T& fill) {
    return Initial{build_filled(0, N, fill)};
  }

  template <std::input_iterator Iter>
  static Initial from_iter(Iter first) {
    return Initial{build_from_iter(0, N, first)};
  }

  template <typename... Args>
  Initial update(size_t index, Args&&... args) const {
    auto new_root =
        updated_node(root.get(), index, std::forward<Args>(args)...);
    return Initial{std::move(new_root)};
  }
};

}  // namespace base