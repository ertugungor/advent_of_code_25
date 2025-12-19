#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

struct Range
{
    long start;
    long end;
};

std::vector<Range> parse_input(std::string_view content)
{
    std::vector<Range> result;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        for (auto range_rng : split(line, ','))
        {
            auto range_sv = to_string_view(range_rng);

            // Parse "start-end" into Range{start, end}
            long start = 0;
            long end = 0;
            int bound_idx = 0;
            for (auto bound_rng : split(range_sv, '-'))
            {
                auto bound = to_string_view(bound_rng);
                if (bound_idx == 0)
                {
                    start = to_int<long>(bound);
                }
                else
                {
                    end = to_int<long>(bound);
                }
                ++bound_idx;
            }
            result.push_back({start, end});
        }
    }

    return result;
}

long long part1(const std::vector<Range>& input)
{
    long long total = 0;
    for (const auto& range : input)
    {
        for (long i = range.start; i <= range.end; ++i)
        {
            std::string num_str = std::to_string(i);
            size_t j = 0;
            size_t j2 = num_str.size() / 2;
            if (num_str.size() % 2 != 0)
            {
                continue;  // Skip odd-length numbers
            }
            while (j2 < num_str.size() && num_str[j] == num_str[j2])
            {
                ++j;
                ++j2;
            }
            if (j2 == num_str.size())
            {
                total += i;
            }
        }
    }
    return total;
}

long long part2(const std::vector<Range>& input)
{
    long long total = 0;
    std::unordered_set<long> invalid_nums;

    for (const auto& range : input)
    {
        std::unordered_map<long, std::unordered_set<std::string>> patterns;
        for (long i = range.start; i <= range.end; ++i)
        {
            std::string num_str = std::to_string(i);
            size_t half_size = num_str.size() / 2;
            for (size_t j = 0; j < half_size; ++j)
            {
                const auto pattern = num_str.substr(0, j + 1);
                patterns[i].insert(pattern);
            }
        }

        for (long i = range.start; i <= range.end; ++i)
        {
            std::string num_str = std::to_string(i);
            for (const auto& pattern : patterns[i])
            {
                size_t pattern_len = pattern.size();
                if (num_str.size() % pattern_len != 0)
                {
                    continue;
                }
                size_t num_idx = 0;
                size_t pattern_idx = 0;
                bool invalid = false;
                while (num_idx < num_str.size())
                {
                    if (num_str[num_idx] != pattern[pattern_idx])
                    {
                        invalid = true;
                        break;
                    }
                    ++num_idx;
                    pattern_idx = (pattern_idx + 1) % pattern_len;
                }
                if (invalid || pattern_idx != 0)
                {
                    continue;
                }

                invalid_nums.insert(i);
            }
        }
    }

    for (const auto& n : invalid_nums)
    {
        total += n;
    }
    return total;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day02/long.txt");
        const auto input = parse_input(content);

        assert(part1(input) == 44487518055);
        assert(part2(input) == 53481866137);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
