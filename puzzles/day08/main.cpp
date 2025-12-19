#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <iostream>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

template <typename T>
concept EdgeType = requires(T edge) {
    { edge.distance } -> std::convertible_to<long long>;
    requires requires(T a, T b) { a <=> b; };
};

struct Vertex
{
    int x, y, z, id;
};

struct Edge
{
    Vertex v1, v2;
    long long distance;

    // C++20 three-way comparison - enables automatic sorting by distance
    auto operator<=>(const Edge& other) const { return distance <=> other.distance; }
};

class UnionFind
{
  public:
    UnionFind(int n) : parent(n), size(n, 1), num_components(n)
    {
        for (int i = 0; i < n; ++i)
        {
            parent[i] = i;
        }
    }

    int find(int x)
    {
        if (parent[x] != x)
        {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    bool unite(int x, int y)
    {
        int root_x = find(x);
        int root_y = find(y);
        if (root_x == root_y)
        {
            return false;
        }
        if (size[root_x] < size[root_y])
        {
            parent[root_x] = root_y;
            size[root_y] += size[root_x];
        }
        else
        {
            parent[root_y] = root_x;
            size[root_x] += size[root_y];
        }
        --num_components;
        return true;
    }

    std::vector<int> get_top_connected_component_sizes(int num_components)
    {
        assert(num_components <= this->num_components && "Requesting more components than exist");
        std::vector<int> size_copy = size;
        std::sort(size_copy.begin(), size_copy.end());
        std::vector<int> result;

        for (int i = 0; i < num_components; ++i)
        {
            result.push_back(size_copy[size_copy.size() - 1 - i]);
        }
        return result;
    }

  private:
    std::vector<int> parent;
    std::vector<int> size;
    int num_components;
};

// Graph with all edges pre-computed and sorted by distance (for Kruskal's algorithm)
template <EdgeType EdgeT = Edge>
struct Graph
{
    std::vector<Vertex> vertices;
    std::vector<EdgeT> edges;
};

Graph<Edge> parse_and_build_graph(std::string_view content)
{
    std::vector<Vertex> vertices;
    int id = 0;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        std::array<int, 3> coords = {0, 0, 0};
        int idx = 0;
        for (auto part : split(line, ','))
        {
            if (idx < 3)
            {
                coords[idx++] = to_int<int>(to_string_view(part));
            }
        }

        vertices.push_back(Vertex{coords[0], coords[1], coords[2], id++});
    }

    std::vector<Edge> edges;
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        for (size_t j = 0; j < i; ++j)
        {
            const auto& v1 = vertices[i];
            const auto& v2 = vertices[j];

            long long dist = static_cast<long long>(v1.x - v2.x) * (v1.x - v2.x) +
                             static_cast<long long>(v1.y - v2.y) * (v1.y - v2.y) +
                             static_cast<long long>(v1.z - v2.z) * (v1.z - v2.z);

            edges.push_back(Edge{v1, v2, dist});
        }
    }

    // Sort edges by distance (Kruskal's algorithm)
    std::sort(edges.begin(), edges.end());

    return Graph{std::move(vertices), std::move(edges)};
}

long long part1(const Graph<Edge>& graph)
{
    UnionFind uf(graph.vertices.size());
    int num_edges_to_add = 1000;
    int num_edges_added = 0;

    for (const auto& edge : graph.edges)
    {
        uf.unite(edge.v1.id, edge.v2.id);
        ++num_edges_added;
        if (num_edges_added >= num_edges_to_add)
        {
            break;
        }
    }

    auto top_group_sizes = uf.get_top_connected_component_sizes(3);
    long long res = 1;
    for (const auto& val : top_group_sizes)
    {
        res *= val;
    }
    return res;
}

long long part2(const Graph<Edge>& graph)
{
    UnionFind uf(graph.vertices.size());
    int num_components = graph.vertices.size();
    long long res = 0;

    for (const auto& edge : graph.edges)
    {
        if (uf.unite(edge.v1.id, edge.v2.id))
        {
            --num_components;
        }
        if (num_components == 1)
        {
            res = static_cast<long long>(edge.v1.x) * edge.v2.x;
            break;
        }
    }
    return res;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day08/long.txt");
        const auto graph = parse_and_build_graph(content);

        assert(part1(graph) == 84968);
        assert(part2(graph) == 8663467782);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
