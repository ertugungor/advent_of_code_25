#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

using Vertex = std::pair<long, long>;  // (row, col)

std::vector<Vertex> parse_input(std::string_view content)
{
    std::vector<Vertex> vertices;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        std::array<long, 2> coords = {0, 0};
        int idx = 0;
        for (auto part : split(line, ','))
        {
            if (idx < 2)
            {
                coords[idx++] = to_int<long>(to_string_view(part));
            }
        }

        // Store as (row, col) but input is col,row
        vertices.push_back({coords[1], coords[0]});
    }

    return vertices;
}

long long part1(const std::vector<Vertex>& positions)
{
    long long max_area = 0;
    for (size_t i = 0; i < positions.size() - 1; ++i)
    {
        for (size_t j = 0; j < positions.size() - 1; ++j)
        {
            if (i == j)
            {
                continue;
            }
            auto [r1, c1] = positions[i];
            auto [r2, c2] = positions[j];
            long long area = std::abs(r1 - r2 + 1) * std::abs(c1 - c2 + 1);
            max_area = std::max(max_area, area);
        }
    }
    return max_area;
}

std::vector<long> get_coords(const std::vector<long>& raw)
{
    std::set<long> coord_set(raw.begin(), raw.end());
    std::vector<long> coords(coord_set.begin(), coord_set.end());
    if (!coords.empty())
    {
        coords.insert(coords.begin(), coords.front() - 1);
        coords.push_back(coords.back() + 1);
    }
    return coords;
}

int get_idx(long coord, const std::vector<long>& coords)
{
    auto it = std::lower_bound(coords.begin(), coords.end(), coord);
    return 2 * std::distance(coords.begin(), it);
}

long long part2(const std::vector<Vertex>& vertices)
{
    std::vector<long> all_rows, all_cols;
    for (const auto& [r, c] : vertices)
    {
        all_rows.push_back(r);
        all_cols.push_back(c);
    }

    auto row_coords = get_coords(all_rows);
    auto col_coords = get_coords(all_cols);

    int height = 2 * row_coords.size();
    int width = 2 * col_coords.size();

    std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));

    int num_vertices = vertices.size();
    for (int i = 0; i < num_vertices; ++i)
    {
        auto [r1, c1] = vertices[i];
        auto [r2, c2] = vertices[(i + 1) % num_vertices];

        int r1_idx = get_idx(r1, row_coords);
        int c1_idx = get_idx(c1, col_coords);
        int r2_idx = get_idx(r2, row_coords);
        int c2_idx = get_idx(c2, col_coords);

        if (r1_idx == r2_idx)
        {
            int c_start = std::min(c1_idx, c2_idx);
            int c_end = std::max(c1_idx, c2_idx);
            for (int k = c_start; k <= c_end; ++k)
            {
                grid[r1_idx][k] = 1;
            }
        }
        else
        {
            int r_start = std::min(r1_idx, r2_idx);
            int r_end = std::max(r1_idx, r2_idx);
            for (int k = r_start; k <= r_end; ++k)
            {
                grid[k][c1_idx] = 1;
            }
        }
    }

    std::vector<std::pair<int, int>> q;
    q.push_back({0, 0});
    grid[0][0] = 2;

    size_t head = 0;
    while (head < q.size())
    {
        std::vector<std::pair<int, int>> four_dirs = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        auto [r, c] = q[head++];

        for (const auto& [dr, dc] : four_dirs)
        {
            int nr = r + dr;
            int nc = c + dc;

            if (nr >= 0 && nr < height && nc >= 0 && nc < width && grid[nr][nc] == 0)
            {
                grid[nr][nc] = 2;
                q.push_back({nr, nc});
            }
        }
    }

    std::vector<std::vector<int>> pref(height + 1, std::vector<int>(width + 1, 0));
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int is_invalid = (grid[i][j] == 2 ? 1 : 0);
            pref[i + 1][j + 1] = is_invalid + pref[i][j + 1] + pref[i + 1][j] - pref[i][j];
        }
    }

    auto get_prefix_sum = [&](int r1, int c1, int r2, int c2) {
        return pref[r2 + 1][c2 + 1] - pref[r1][c2 + 1] - pref[r2 + 1][c1] + pref[r1][c1];
    };

    long long max_area = std::numeric_limits<long long>::min();

    for (int i = 0; i < num_vertices; ++i)
    {
        for (int j = i + 1; j < num_vertices; ++j)
        {
            auto [r1, c1] = vertices[i];
            auto [r2, c2] = vertices[j];

            int r1_idx = get_idx(r1, row_coords);
            int c1_idx = get_idx(c1, col_coords);
            int r2_idx = get_idx(r2, row_coords);
            int c2_idx = get_idx(c2, col_coords);

            int r_min = std::min(r1_idx, r2_idx);
            int r_max = std::max(r1_idx, r2_idx);
            int c_min = std::min(c1_idx, c2_idx);
            int c_max = std::max(c1_idx, c2_idx);

            if (get_prefix_sum(r_min, c_min, r_max, c_max) == 0)
            {
                long long area = (std::abs(r1 - r2) + 1) * (std::abs(c1 - c2) + 1);
                max_area = std::max(max_area, area);
            }
        }
    }
    return max_area;
}

long long part2_naive(const std::vector<Vertex>& vertices)
{
    long long max_area = 0;
    int n = vertices.size();

    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            long r1 = vertices[i].first;
            long c1 = vertices[i].second;
            long r2 = vertices[j].first;
            long c2 = vertices[j].second;

            long min_r = std::min(r1, r2);
            long max_r = std::max(r1, r2);
            long min_c = std::min(c1, c2);
            long max_c = std::max(c1, c2);

            double mid_r = (min_r + max_r) / 2.0;
            double mid_c = (min_c + max_c) / 2.0;
            int intersections = 0;

            for (int k = 0; k < n; ++k)
            {
                long vr1 = vertices[k].first;
                long vc1 = vertices[k].second;
                long vr2 = vertices[(k + 1) % n].first;
                long vc2 = vertices[(k + 1) % n].second;

                if ((vr1 > mid_r) != (vr2 > mid_r))
                {
                    double intersect_c = vc1 + (double)(mid_r - vr1) * (vc2 - vc1) / (vr2 - vr1);
                    if (intersect_c > mid_c)
                    {
                        intersections++;
                    }
                }
            }

            if (intersections % 2 == 0)
            {
                continue;
            }

            bool edge_intersects = false;
            for (int k = 0; k < n; ++k)
            {
                long vr1 = vertices[k].first;
                long vc1 = vertices[k].second;
                long vr2 = vertices[(k + 1) % n].first;
                long vc2 = vertices[(k + 1) % n].second;

                if (vr1 == vr2)
                {
                    long edge_r = vr1;
                    long edge_c_min = std::min(vc1, vc2);
                    long edge_c_max = std::max(vc1, vc2);

                    if (edge_r > min_r && edge_r < max_r)
                    {
                        if (std::max(edge_c_min, min_c) < std::min(edge_c_max, max_c))
                        {
                            edge_intersects = true;
                            break;
                        }
                    }
                }
                else
                {
                    long edge_c = vc1;
                    long edge_r_min = std::min(vr1, vr2);
                    long edge_r_max = std::max(vr1, vr2);

                    if (edge_c > min_c && edge_c < max_c)
                    {
                        if (std::max(edge_r_min, min_r) < std::min(edge_r_max, max_r))
                        {
                            edge_intersects = true;
                            break;
                        }
                    }
                }
            }

            if (!edge_intersects)
            {
                long long area = (long long)(max_r - min_r + 1) * (max_c - min_c + 1);
                max_area = std::max(max_area, area);
            }
        }
    }
    return max_area;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day09/long.txt");
        const auto vertices = parse_input(content);

        assert(part1(vertices) == 4763040296);
        assert(part2_naive(vertices) == 1396494456);
        assert(part2(vertices) == 1396494456);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
