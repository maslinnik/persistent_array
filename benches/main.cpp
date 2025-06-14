#include <benchmark/benchmark.h>
#include <random>
#include "persistent_array.h"

template <typename T, size_t N, template <typename> typename Base>
static void StoredRandomUpdates(benchmark::State& state) {
  using pa_t = persistent_array<T, Base<T>>;

  std::mt19937 rnd{};
  std::vector<pa_t> v = {pa_t(N)};

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

BENCHMARK(StoredRandomUpdates<int, 1000, base::Initial>);
BENCHMARK(StoredRandomUpdates<int, 1000, base::MySharedPtr>);
BENCHMARK(StoredRandomUpdates<int, 1000, base::FourFold>);
BENCHMARK(StoredRandomUpdates<int, 1000, base::EightFold>);

template <typename T, size_t N, template <typename> typename Base>
static void CumulativeRandomUpdates(benchmark::State& state) {
  using pa_t = persistent_array<T, Base<T>>;

  std::mt19937 rnd{};
  pa_t pa(N);

  for (auto _ : state) {
    int position = rnd() % N;
    int new_val = rnd();

    pa = pa.update(position, new_val);
  }
}

BENCHMARK(CumulativeRandomUpdates<int, 1000, base::Initial>);
BENCHMARK(CumulativeRandomUpdates<int, 1000, base::MySharedPtr>);
BENCHMARK(CumulativeRandomUpdates<int, 1000, base::FourFold>);
BENCHMARK(CumulativeRandomUpdates<int, 1000, base::EightFold>);

template <typename T, size_t N, template <typename> typename Base>
static void Traversal(benchmark::State& state) {
  using pa_t = persistent_array<T, Base<T>>;

  std::mt19937 rnd{};
  pa_t pa(N);
  for (int i = 0; i < 2 * N; ++i) {
    int position = rnd() % N;
    int new_val = rnd();
    pa = pa.update(position, new_val);
  }

  for (auto _ : state) {
    auto it = pa.begin();
    for (int i = 0; i < N; ++i) {
      ++it;
    }
  }
}

BENCHMARK(Traversal<int, 1000, base::Initial>);
BENCHMARK(Traversal<int, 1000, base::MySharedPtr>);
BENCHMARK(Traversal<int, 1000, base::FourFold>);
BENCHMARK(Traversal<int, 1000, base::EightFold>);

template <typename T, size_t N, template <typename> typename Base>
static void Indexing(benchmark::State& state) {
  using pa_t = persistent_array<T, Base<T>>;

  std::mt19937 rnd{};
  pa_t pa(N);
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
BENCHMARK(Indexing<int, 1000, base::FourFold>);
BENCHMARK(Indexing<int, 1000, base::EightFold>);

BENCHMARK_MAIN();