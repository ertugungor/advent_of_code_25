#include <cassert>
#include <iostream>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

// Helper for floor division (rounds towards negative infinity)
// Used to handle negative coordinates correctly.
/*
 * edge cases for left turns crossing zero:
 * start = 101, val = 102   => floor_div(100, 100) - floor_div(-2, 100)   = 1  - (-1) = 2
 * start = 1, val = 2       => floor_div(0, 100)   - floor_div(-2, 100)   = 0  - (-1) = 1
 * start = 1, val = 1       => floor_div(0, 100)   - floor_div(-1,100)    = 0  - (-1) = 1
 * start = 30, val = 32     => floor_div(29, 100)  - floor_div(-3, 100)   = 0  - (-1) = 1
 * start = 30, val = 28     => floor_div(30, 100)  - floor_div(1, 100)    = 0  - 0    = 0
 * start = 30, val = 330    => floor_div(29, 100)  - floor_div(-301, 100) = 0  - (-4) = 0
 * start = 0, val = 10      => floor_div(-1, 100)  - floor_div(-11, 100)  = -1 - (-1) = 0
 */

constexpr int floor_div(int a, int b)
{
    int res = a / b;
    // In C++, % can return negative values if a is negative.
    // If there is a remainder and the result is negative (due to a < 0),
    // we need to subtract 1 to floor it.
    // Since b is always 100 here, we just check if remainder is negative.
    if (a % b < 0)
    {
        --res;
    }
    return res;
}

struct Instruction
{
    char turn;
    int distance;
};

std::vector<Instruction> parse_instructions(std::string_view content)
{
    std::vector<Instruction> instructions;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        instructions.push_back({line[0], to_int<int>(line.substr(1))});
    }

    return instructions;
}

int part1(const std::vector<Instruction>& instructions)
{
    int start = 50;
    int count = 0;

    for (const auto& [rot, val] : instructions)
    {
        start = rot == 'L' ? start - val : start + val;
        start = rot == 'L' ? (start % 100 + 100) % 100 : start % 100;
        count = start == 0 ? count + 1 : count;
    }
    return count;
}

int part2(const std::vector<Instruction>& instructions)
{
    int start = 50;
    int count = 0;
    for (const auto& [rot, val] : instructions)
    {
        {
            if (rot == 'L')
            {
                // floor_div is used to make C++ negative division rounds down, rather than
                // truncating toward zero
                count += floor_div(start - 1, 100) - floor_div(start - val - 1, 100);
                start -= val;
                start = (start % 100 + 100) % 100;  // negative modulo
            }
            else if (rot == 'R')
            {
                start += val;
                count += start / 100;
                start = start % 100;
            }
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
        const auto content = read_file("puzzles/day01/long.txt");
        const auto instructions = parse_instructions(content);

        assert(part1(instructions) == 1154);
        assert(part2(instructions) == 6819);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
