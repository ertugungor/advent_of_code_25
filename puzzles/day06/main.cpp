#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

struct Input
{
    std::vector<std::string> lines;
    std::vector<char> ops;
};

Input parse_input(std::string_view content)
{
    std::vector<std::string> lines;
    std::vector<char> ops;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }
        lines.push_back(std::string(line));
    }

    // Last line contains operators
    if (!lines.empty())
    {
        std::istringstream ss(lines.back());
        char op;
        while (ss >> op)
        {
            ops.push_back(op);
        }
    }

    return {std::move(lines), std::move(ops)};
}

long long part1(const Input& input)
{
    std::vector<std::vector<long long>> numbers;

    // Parse numbers from all lines except the last (operator line)
    for (size_t i = 0; i < input.lines.size() - 1; ++i)
    {
        std::istringstream ss(input.lines[i]);
        long long num;
        std::vector<long long> row;
        while (ss >> num)
        {
            row.push_back(num);
        }
        numbers.push_back(row);
    }

    long long total_sum = 0;
    size_t cols = numbers[0].size();

    for (size_t c = 0; c < cols; ++c)
    {
        long long res = input.ops[c] == '+' ? 0 : 1;
        for (size_t r = 0; r < numbers.size(); ++r)
        {
            if (c < numbers[r].size())
            {
                if (input.ops[c] == '+')
                {
                    res += numbers[r][c];
                }
                else if (input.ops[c] == '*')
                {
                    res *= numbers[r][c];
                }
            }
        }
        total_sum += res;
    }

    return total_sum;
}

long long part2(const Input& input)
{
    int col_idx = 0;
    size_t max_empty_char_idx = 0;
    int op_idx = 0;
    long long total_sum = 0;

    while (true)
    {
        for (size_t i = 0; i < input.lines.size() - 1; ++i)
        {
            size_t empty_char_idx = input.lines[i].find(' ', col_idx);
            max_empty_char_idx = std::max(max_empty_char_idx, empty_char_idx);
        }

        std::vector<std::string> nums;
        for (size_t i = 0; i < input.lines.size() - 1; ++i)
        {
            std::string num_str = input.lines[i].substr(col_idx, max_empty_char_idx - col_idx);
            std::reverse(num_str.begin(), num_str.end());
            nums.push_back(num_str);
        }

        size_t col_size = nums[0].size();
        char op = input.lines.back()[op_idx];
        op_idx = max_empty_char_idx + 1;
        long long res = op == '+' ? 0 : 1;

        for (size_t c = 0; c < col_size; ++c)
        {
            std::string digit_str;
            for (const auto& num : nums)
            {
                digit_str += num[c];
            }

            std::istringstream ss(digit_str);
            long long val;
            ss >> val;

            if (op == '+')
            {
                res += val;
            }
            else if (op == '*')
            {
                res *= val;
            }
        }
        total_sum += res;

        if (max_empty_char_idx == std::string::npos)
        {
            break;
        }
        col_idx = max_empty_char_idx + 1;
        max_empty_char_idx = 0;
    }

    return total_sum;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day06/long.txt");
        const auto input = parse_input(content);

        assert(part1(input) == 4805473544166);
        assert(part2(input) == 8907730960817);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
