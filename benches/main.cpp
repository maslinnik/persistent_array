#include <benchmark/benchmark.h>
#include <random>
#include "persistent_array.h"
#include "versions/my_shared_ptr.h"

template <typename T, size_t N, template <typename, size_t> typename Base>
static void RandomUpdates(benchmark::State& state) {
  using pa_t = persistent_array<T, N, Base<T, N>>;

  std::mt19937 rnd{};
  std::vector<pa_t> v = {pa_t{0}};

  for (auto _ : state) {
    int index = rnd() % v.size();
    int position = rnd() % N;
    int new_val = rnd();

    auto new_version = v[index].update(position, new_val);

    state.PauseTiming();
    v.push_back(new_version);
    state.ResumeTiming();
  }
}

BENCHMARK(RandomUpdates<int, 1000, base::Initial>);
BENCHMARK(RandomUpdates<int, 1000, base::MySharedPtr>);

template <typename T, size_t N, template <typename, size_t> typename Base>
static void Traversal(benchmark::State& state) {
  using pa_t = persistent_array<T, N, Base<T, N>>;

  std::mt19937 rnd{};
  pa_t pa{0};
  for (int i = 0; i < 2 * N; ++i) {
    int position = rnd() % N;
    int new_val = rnd();
    pa = pa.update(position, new_val);
  }

  for (auto _ : state) {
    for (int x : pa) {}
  }
}

BENCHMARK(Traversal<int, 1000, base::Initial>);
BENCHMARK(Traversal<int, 1000, base::MySharedPtr>);

template <typename T, size_t N, template <typename, size_t> typename Base>
static void Indexing(benchmark::State& state) {
  using pa_t = persistent_array<T, N, Base<T, N>>;

  std::mt19937 rnd{};
  pa_t pa{0};
  for (int i = 0; i < 2 * N; ++i) {
    int position = rnd() % N;
    int new_val = rnd();
    pa = pa.update(position, new_val);
  }

  for (auto _ : state) {
    int position = rnd() % N;
    pa[position];
  }
}

BENCHMARK(Indexing<int, 1000, base::Initial>);
BENCHMARK(Indexing<int, 1000, base::MySharedPtr>);

BENCHMARK_MAIN();