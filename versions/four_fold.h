#include <numeric>

#include "../inplace_vector"

namespace base {

template <typename T, size_t N>
struct FourFold {
  struct BaseNode {
    size_t size;
    size_t ref_count = 1;
  };

  class Rc;

  struct IntermediateNode : BaseNode {
    std::array<Rc, 4> children;

    IntermediateNode(size_t size, std::array<Rc, 4> c)
        : BaseNode(size),
          children(std::move(c)) {}
  };

  struct DataNode : BaseNode {
    T x;

    template <typename... Args>
    DataNode(Args&&... args) : BaseNode(1), x(std::forward<Args>(args)...) {}
  };

  class Rc {
   private:
    BaseNode* ptr = nullptr;

    Rc(BaseNode* raw) : ptr(raw) {}

   public:
    Rc(const Rc& rc) : ptr(rc.ptr) {
      if (!ptr) {
        return;
      }
      ptr->ref_count += 1;
    }

    Rc(Rc&& rc) noexcept : ptr(rc.ptr) { rc.ptr = nullptr; }

    void swap(Rc& rc) { std::swap(ptr, rc.ptr); }

    Rc& operator=(const Rc& rc) {
      Rc{rc}.swap(*this);
      return *this;
    }

    Rc& operator=(Rc&& rc) noexcept {
      Rc{std::move(rc)}.swap(*this);
      return *this;
    }

    ~Rc() {
      if (!ptr) {
        return;
      }
      ptr->ref_count -= 1;
      if (ptr->ref_count == 0) {
        if (ptr->size == 1) {
          delete static_cast<DataNode*>(ptr);
        } else {
          delete static_cast<IntermediateNode*>(ptr);
        }
      }
    }

    BaseNode* operator->() const { return ptr; }

    BaseNode& operator*() const { return *ptr; }

    BaseNode* get() const { return ptr; }

    template <typename... Args>
    static Rc make_base(Args&&... args) {
      return {new DataNode(std::forward<Args>(args)...)};
    }

    static Rc make_intermediate(size_t size, std::array<Rc, 4> c) {
      return {new IntermediateNode(size, std::move(c))};
    }

    Rc() = default;
  };

  Rc root;

  static size_t which(size_t i, size_t n) {
    return i / ((n + 3) / 4);
  }

  static size_t child_size(size_t n) {
    return (n + 3) / 4;
  }

  template <bool IsConst>
  class BaseIterator {
    static const size_t STACK_SIZE = N == 1 ? 2 : (std::bit_width(N - 1) + 3) / 2;
    using StackType = std::inplace_vector<BaseNode*, STACK_SIZE>;

    StackType stack;
    uint64_t mask = 0;

    friend struct FourFold;

    void go_to_kth(size_t k) {
      while (stack.back()->size > 1) {
        auto intermediate_node = static_cast<IntermediateNode*>(stack.back());
        size_t index = which(k, stack.back()->size);
        mask |= index << (2 * stack.size() - 2);
        k -= child_size(stack.back()->size) * index;
        stack.push_back(intermediate_node->children[index].get());
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
        size_t index = mask >> (2 * stack.size() - 4) & 3;
        k += child_size(parent->size) * index;
        stack.pop_back();
        mask &= ~(3 << (2 * stack.size() - 2));
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
      auto lca_index = [&](const StackType& stack, uint64_t mask) {
        if (!stack.back()) {
          return N;
        }
        size_t result = 0;
        for (size_t i = lca_depth; i + 1 < stack.size(); ++i) {
          auto intermediate_node = static_cast<IntermediateNode*>(stack[i]);
          size_t index = mask >> (2 * i) & 3;
          result += child_size(intermediate_node->size) * index;
        }
        return result;
      };
      return lca_index(stack, mask) - lca_index(other.stack, other.mask);
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

  explicit FourFold(Rc root) : root(std::move(root)) {}

  BaseIterator<true> begin() const { return {root.get(), 0}; }

  BaseIterator<true> end() const { return {root.get(), N}; }

  BaseIterator<false> mutable_begin() { return {root.get(), 0}; }

  BaseIterator<false> mutable_end() { return {root.get(), N}; }

  template <std::input_iterator Iter>
  static Rc build_from_iter(size_t l, size_t r, Iter& iter) {
    if (l == r) {
      return Rc{};
    } else if (l + 1 == r) {
      return Rc::make_base(*iter++);
    } else {
      size_t size = r - l;
      std::array<Rc, 4> children{};
      for (int i = 0; i < 4; ++i) {
        children[i] = build_from_iter(std::min(r, l + child_size(size) * i), std::min(r, l + child_size(size) * (i + 1)), iter);
      }
      return Rc::make_intermediate(size, std::move(children));
    }
  }

  static Rc build_filled(size_t l, size_t r, const T& fill) {
    if (l + 1 == r) {
      return Rc::make_base(fill);
    } else {
      size_t size = r - l;
      std::array<Rc, 4> children{};
      for (int i = 0; i < 4; ++i) {
        children[i] = build_from_iter(std::min(r, l + child_size(size) * i), std::min(r, l + child_size(size) * (i + 1)), fill);
      }
      return Rc::make_intermediate(size, std::move(children));
    }
  }

  template <typename... Args>
  Rc updated_node(BaseNode* curr, size_t i, Args&&... args) const {
    if (curr->size == 1) {
      return Rc::make_base(std::forward<Args>(args)...);
    }
    auto intermediate_node = static_cast<IntermediateNode*>(curr);
    std::array<Rc, 4> new_children{intermediate_node->children};
    size_t index = which(i, curr->size);
    i -= child_size(curr->size) * index;
    new_children[index] = updated_node(new_children[index].get(), i, std::forward<Args>(args)...);
    return Rc::make_intermediate(curr->size, std::move(new_children));
  }

  static FourFold filled(const T& fill) {
    return FourFold{std::move(build_filled(0, N, fill))};
  }

  template <std::input_iterator Iter>
  static FourFold from_iter(Iter first) {
    return FourFold{std::move(build_from_iter(0, N, first))};
  }

  template <typename... Args>
  FourFold update(size_t index, Args&&... args) const {
    auto new_root =
        updated_node(root.get(), index, std::forward<Args>(args)...);
    return FourFold{std::move(new_root)};
  }
};

}  // namespace base