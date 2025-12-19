#pragma once

#include <charconv>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>

namespace aoc
{

// Reads the entire file into a std::string.
// This string will be the "Owner" of the data.
// All views (string_view) must not outlive this string.
inline std::string read_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Could not open file: " + path.string());
    }

    auto size = file.tellg();
    std::string content(size, '\0');
    file.seekg(0);

    if (!file.read(&content[0], size))
    {
        throw std::runtime_error("Could not read file: " + path.string());
    }
    return content;
}

// Splits a string_view by a delimiter.
// Returns a view of subranges.
// Usage: for (auto part : split(sv, ',')) { ... }
constexpr auto split(std::string_view s, char delim)
{
    return s | std::views::split(delim);
}

// Helper to convert a range (like from split) to string_view.
// Note: We cannot strictly require std::ranges::contiguous_range<R> here because
// std::views::split returns a view whose iterators are not technically "contiguous"
// (they calculate the split on the fly), even if the underlying data is.
// However, we can still extract the address of the first element.
template <typename R>
    requires std::same_as<std::ranges::range_value_t<R>, char>
constexpr std::string_view to_string_view(R&& r)
{
    auto b = std::ranges::begin(r);
    auto e = std::ranges::end(r);
    if (b == e)
        return {};
    // We assume the underlying data is contiguous (it is for string_view split)
    return std::string_view(&*b, std::ranges::distance(r));
}

// Returns a view over lines in the string.
// Usage: for (auto line_rng : get_lines(content)) { ... }
constexpr auto get_lines(std::string_view s)
{
    return split(s, '\n');
}

// Helper to convert a range (like a part from split) to a std::string.
template <typename Range>
std::string to_string(Range&& r)
{
    return std::string(std::ranges::begin(r), std::ranges::end(r));
}

// High-performance integer parsing using std::from_chars (C++17)
// Works with string_view directly.
template <typename T = int>
    requires std::integral<T>
T to_int(std::string_view sv)
{
    T result;
    // Note: sv.end() might not be null-terminated, but from_chars takes a range [first, last).
    // We also need to handle potential leading whitespace if the split leaves it,
    // though std::from_chars does NOT skip whitespace by default.
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

    if (ec != std::errc())
    {
        throw std::runtime_error("Failed to parse integer: " + std::string(sv));
    }
    return result;
}

}  // namespace aoc
