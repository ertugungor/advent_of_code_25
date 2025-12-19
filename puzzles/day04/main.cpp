#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

constexpr std::array<std::pair<int, int>, 8> DIRECTIONS = {{
    {0, 1},    // right
    {1, 0},    // down
    {1, 1},    // down-right
    {1, -1},   // down-left
    {0, -1},   // left
    {-1, 0},   // up
    {-1, -1},  // up-left
    {-1, 1}    // up-right
}};

struct Grid
{
    std::vector<std::string_view> lines;
    int rows;
    int cols;

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
    }

    const std::string_view& operator[](int i) const { return lines[i]; }
};

std::vector<std::string> parse_mutable_grid(std::string_view content)
{
    std::vector<std::string> result;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (!line.empty())
        {
            result.push_back(std::string(line));
        }
    }

    return result;
}

int part1(const Grid& grid)
{
    int num_accessible = 0;
    for (int i = 0; i < grid.rows; ++i)
    {
        for (int j = 0; j < grid.cols; ++j)
        {
            if (grid[i][j] == '.')
            {
                continue;
            }
            int num_papers = 0;
            for (const auto& [di, dj] : DIRECTIONS)
            {
                int x = i + di, y = j + dj;
                if ((x < 0 || x >= grid.rows || y < 0 || y >= grid.cols))
                {
                    continue;
                }
                if (grid[x][y] == '@')
                {
                    ++num_papers;
                }
            }
            if (num_papers < 4)
            {
                ++num_accessible;
            }
        }
    }
    return num_accessible;
}

void dfs(std::vector<std::string>& grid, int x, int y, int rows, int cols, int& num_removed)
{
    if (x < 0 || x >= rows || y < 0 || y >= cols || grid[x][y] == '.' || grid[x][y] == 'x')
    {
        return;
    }

    const auto is_removable = [&]() {
        int num_papers = 0;
        for (const auto& [di, dj] : DIRECTIONS)
        {
            int nx = x + di, ny = y + dj;
            if (nx < 0 || nx >= rows || ny < 0 || ny >= cols)
            {
                continue;
            }
            if (grid[nx][ny] == '@')
            {
                ++num_papers;
            }
        }
        return num_papers < 4;
    };

    if (is_removable())
    {
        grid[x][y] = 'x';
        ++num_removed;
        for (const auto& [di, dj] : DIRECTIONS)
        {
            dfs(grid, x + di, y + dj, rows, cols, num_removed);
        }
    }
}

int part2(std::string_view content)
{
    auto grid = parse_mutable_grid(content);
    int rows = grid.size();
    int cols = grid[0].size();

    int num_removed = 0;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (grid[i][j] == '.' || grid[i][j] == 'x')
            {
                continue;
            }
            dfs(grid, i, j, rows, cols, num_removed);
        }
    }
    return num_removed;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day04/long.txt");

        assert(part1(Grid{content}) == 1533);
        assert(part2(content) == 9206);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
