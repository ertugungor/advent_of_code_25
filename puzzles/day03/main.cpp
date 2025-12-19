#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

template <typename T>
concept MemoTable = requires(T memo, size_t i, size_t j) {
    { memo(i, j) } -> std::convertible_to<long long&>;  // 2D access
};

class Matrix
{
  private:
    std::vector<long long> data;
    size_t cols_;

  public:
    // Matrix provides 2D access to a flat contiguous vector for better cache
    // performance compared to std::vector<std::vector<>> which has scattered
    // allocations.
    Matrix(size_t rows, size_t cols) : data(rows * cols, -1), cols_(cols) {}
    long long& operator()(size_t i, size_t j) { return data[i * cols_ + j]; }
    const long long& operator()(size_t i, size_t j) const { return data[i * cols_ + j]; }
};

constexpr int to_digit(char c)
{
    assert(c >= '0' && c <= '9' && "Character must be a digit");
    return c - '0';
}

class KnapsackSolver
{
  public:
    /**
     * @brief Solves digit selection knapsack problem.
     *
     * Given a string of digits and a number of slots, selects digits to maximize
     * the resulting number. Each digit contributes: digit * 10^(remaining_slots-1)
     *
     * Algorithm: DP with memoization
     * - State: (index, slots_remaining)
     * - Decision: include or exclude current digit
     * - Optimization: Memoize to avoid recomputation
     *
     * Complexity: O(n * s) where n = string length, s = num_slots
     */
    template <MemoTable M>
    long long digit_knapsack(size_t idx, std::string_view nums, int num_slots, M& memo)
    {
        if (idx >= nums.size() || num_slots <= 0)
        {
            return 0;
        }
        if (memo(idx, num_slots) != -1)
        {
            return memo(idx, num_slots);
        }
        int digit = to_digit(nums[idx]);
        long long val = std::pow(10, std::min(num_slots - 1, static_cast<int>(nums.size() - idx - 1))) * digit;

        long long include = val + digit_knapsack(idx + 1, nums, num_slots - 1, memo);
        long long exclude = digit_knapsack(idx + 1, nums, num_slots, memo);
        memo(idx, num_slots) = std::max(include, exclude);
        return memo(idx, num_slots);
    }

    long long solve_line(const std::string_view line, const size_t num_slots)
    {
        size_t n = line.size();
        Matrix memo(n + 1, num_slots + 1);
        return digit_knapsack(0, line, num_slots, memo);
    }
};

long long solve(std::string_view content, size_t num_slots)
{
    KnapsackSolver solver;
    long long sum = 0;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        long long max_num = solver.solve_line(line, num_slots);
        sum += max_num;
    }

    return sum;
}

long long part1(std::string_view content)
{
    return solve(content, 2);
}

long long part2(std::string_view content)
{
    return solve(content, 12);
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day03/long.txt");

        assert(part1(content) == 17435);
        assert(part2(content) == 172886048065379);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
