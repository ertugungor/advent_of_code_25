#include <cassert>
#include <iostream>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

struct Problem
{
    int grid_area;
    int shapes_area;
};

std::vector<Problem> parse_input(std::string_view content)
{
    std::vector<int> shape_sizes;
    std::vector<Problem> problems;

    int current_shape_size = 0;
    bool in_shape = false;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);

        if (line.empty())
        {
            if (in_shape)
            {
                shape_sizes.push_back(current_shape_size);
                in_shape = false;
            }
            continue;
        }

        // Parse grid: "HxW: n1 n2 n3..."
        if (auto x_pos = line.find('x'); x_pos != std::string_view::npos)
        {
            auto colon_pos = line.find(':');
            int grid_area =
                to_int<int>(line.substr(0, x_pos)) * to_int<int>(line.substr(x_pos + 1, colon_pos - x_pos - 1));

            int shapes_area = 0;
            size_t idx = 0;
            for (auto num : split(line.substr(colon_pos + 2), ' '))
            {
                if (auto num_sv = to_string_view(num); !num_sv.empty() && idx < shape_sizes.size())
                {
                    shapes_area += shape_sizes[idx++] * to_int<int>(num_sv);
                }
            }

            problems.push_back({grid_area, shapes_area});
        }
        // Parse shape header: "N:"
        else if (line.size() >= 2 && line[1] == ':')
        {
            in_shape = true;
            current_shape_size = 0;
        }
        // Count '#' in shape
        else if (in_shape)
        {
            for (char c : line)
            {
                current_shape_size += (c == '#');
            }
        }
    }

    return problems;
}

int solve(const std::vector<Problem>& problems)
{
    int count = 0;
    for (const auto& [grid_area, shapes_area] : problems)
    {
        if (grid_area >= shapes_area)
        {
            ++count;
        }
    }
    return count;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day12/long.txt");
        const auto problems = parse_input(content);

        int result = solve(problems);
        std::cout << "Total YES: " << result << " out of " << problems.size() << std::endl;

        // The troll problem: just check if grid area >= shapes area!
        assert(result == 550);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
