#include <benchmark/benchmark.h>
#include "persistent_array.h"
#include <random>

template <typename T, size_t N>
struct SlowPersistentArray {
    std::array<T, N> array;

    const T& operator[](size_t i) const { return array[i]; }

    template <typename... Args>
    SlowPersistentArray update(size_t index, Args&&... args) const {
        auto new_array = array;
        new_array[index] = T{std::forward<Args>(args)...};
        return SlowPersistentArray{new_array};
    }
};

const int N = 1'000;

static void SlowRandomUpdates(benchmark::State& state) {
    std::mt19937 rnd{};
    std::array<int, N> initial{};
    std::vector<SlowPersistentArray<int, N>> v = {{initial}};

    for (auto _ : state) {
        int index = rnd() % v.size();
        int position = rnd() % N;
        int new_val = rnd();
        v.push_back(v[index].update(position, new_val));
    }
}
BENCHMARK(SlowRandomUpdates);

static void FastRandomUpdates(benchmark::State& state) {
    std::mt19937 rnd{};
    std::array<int, N> initial{};
    std::vector<persistent_array<int, N>> v = {persistent_array<int, N>{initial.begin()}};

    for (auto _ : state) {
        int index = rnd() % v.size();
        int position = rnd() % N;
        int new_val = rnd();
        v.push_back(v[index].update(position, new_val));
    }
}
BENCHMARK(FastRandomUpdates);

BENCHMARK_MAIN();