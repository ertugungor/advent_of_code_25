#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

template <typename T>
concept CharGridLike = requires(T grid, int row, int col) {
    { grid.rows } -> std::convertible_to<int>;
    { grid.cols } -> std::convertible_to<int>;
    { grid.start_row } -> std::convertible_to<int>;
    { grid.start_col } -> std::convertible_to<int>;
    { grid(row, col) } -> std::convertible_to<char>;
};

struct Grid
{
    std::vector<std::string_view> lines;
    int rows;
    int cols;
    int start_row;
    int start_col;

    /**
     * @brief Constructs grid from text content and locates start position 'S'.
     *
     * Parses line-separated content into string_view rows for zero-copy access.
     * Scans grid to find 'S' character and stores its coordinates.
     */
    Grid(std::string_view content)
    {
        for (auto line_rng : get_lines(content))
        {
            auto line = to_string_view(line_rng);
            if (!line.empty())
            {
                lines.push_back(line);
            }
        }
        rows = lines.size();
        cols = rows > 0 ? lines[0].size() : 0;

        // Find starting position 'S'
        start_row = 0;
        start_col = 0;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                if (lines[r][c] == 'S')
                {
                    start_row = r;
                    start_col = c;
                    return;
                }
            }
        }
    }

    constexpr char operator()(int row, int col) const
    {
        assert(row >= 0 && row < rows && col >= 0 && col < cols && "Grid access out of bounds");
        return lines[row][col];
    }
};

template <CharGridLike GridT>
long long part1(const GridT& grid)
{
    std::vector<int> active_columns(grid.cols, 0);
    active_columns[grid.start_col] = 1;

    int hit_count = 0;
    for (int i = grid.start_row + 1; i < grid.rows; ++i)
    {
        std::vector<int> next_active_columns(grid.cols, 0);

        for (int j = 0; j < grid.cols; ++j)
        {
            if (active_columns[j] == 1)
            {
                if (grid(i, j) == '^')
                {
                    ++hit_count;
                    if (j - 1 >= 0)
                    {
                        next_active_columns[j - 1] = 1;
                    }
                    if (j + 1 < grid.cols)
                    {
                        next_active_columns[j + 1] = 1;
                    }
                }
                else if (grid(i, j) == '.')
                {
                    next_active_columns[j] = 1;
                }
            }
        }

        active_columns = std::move(next_active_columns);
    }

    return hit_count;
}

template <CharGridLike GridT>
long long part2(const GridT& grid)
{
    std::vector<long long> counts(grid.cols, 0);
    counts[grid.start_col] = 1;

    for (int i = grid.start_row + 1; i < grid.rows; ++i)
    {
        std::vector<long long> next_counts(grid.cols, 0);

        for (int j = 0; j < grid.cols; ++j)
        {
            if (counts[j] == 0)
            {
                continue;
            }

            if (grid(i, j) == '^')
            {
                if (j - 1 >= 0)
                {
                    next_counts[j - 1] += counts[j];
                }
                if (j + 1 < grid.cols)
                {
                    next_counts[j + 1] += counts[j];
                }
            }
            else
            {
                next_counts[j] += counts[j];
            }
        }

        counts = std::move(next_counts);
    }

    return std::accumulate(counts.begin(), counts.end(), 0LL);
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day07/long.txt");
        const Grid grid{content};

        assert(part1(grid) == 1642);
        assert(part2(grid) == 47274292756692);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
