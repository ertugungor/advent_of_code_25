#include <cassert>
#include <iostream>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

struct RawInput
{
    std::vector<std::pair<long long, long long>> ranges;
    std::vector<long long> ids;
};

struct Input
{
    std::vector<std::pair<long long, long long>> merged_ranges;
    std::vector<long long> ids;
};

RawInput parse_input(std::string_view content)
{
    std::vector<std::pair<long long, long long>> ranges;
    std::vector<long long> ids;
    bool push_ranges = true;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            push_ranges = false;
            continue;
        }

        if (push_ranges)
        {
            long long start = 0, end = 0;
            int idx = 0;
            for (auto part_rng : split(line, '-'))
            {
                auto part = to_string_view(part_rng);
                if (idx == 0)
                {
                    start = to_int<long long>(part);
                }
                else
                {
                    end = to_int<long long>(part);
                }
                ++idx;
            }
            ranges.emplace_back(start, end);
        }
        else
        {
            ids.push_back(to_int<long long>(line));
        }
    }

    return {ranges, ids};
}

std::vector<std::pair<long long, long long>> merge_ranges(std::vector<std::pair<long long, long long>> ranges)
{
    std::sort(ranges.begin(), ranges.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    std::vector<std::pair<long long, long long>> merged;
    merged.reserve(ranges.size());

    for (const auto& range : ranges)
    {
        if (merged.empty() || merged.back().second < range.first - 1)
        {
            merged.push_back(range);
        }
        else
        {
            merged.back().second = std::max(merged.back().second, range.second);
        }
    }

    return merged;
}

Input preprocess_input(RawInput raw)
{
    return {merge_ranges(std::move(raw.ranges)), std::move(raw.ids)};
}

int part1(const Input& input)
{
    int num_fresh_ids = 0;
    for (const auto& id : input.ids)
    {
        bool is_valid = false;
        for (const auto& [start, end] : input.merged_ranges)
        {
            if (id >= start && id <= end)
            {
                is_valid = true;
                break;
            }
            if (id < start)
            {
                break;
            }
        }
        if (is_valid)
        {
            ++num_fresh_ids;
        }
    }
    return num_fresh_ids;
}

long long part2(const Input& input)
{
    long long total_size = 0;
    for (const auto& [start, end] : input.merged_ranges)
    {
        total_size += (end - start + 1);
    }
    return total_size;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day05/long.txt");
        const auto raw = parse_input(content);
        const auto input = preprocess_input(raw);

        assert(part1(input) == 885);
        assert(part2(input) == 348115621205535);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
