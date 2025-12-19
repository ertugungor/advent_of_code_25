#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

template <typename T>
concept GraphNode = std::equality_comparable<T> && requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
};

template <GraphNode NodeType = std::string>
struct Graph
{
    using NodeId = int;

    std::vector<std::vector<NodeId>> adj_list;
    std::unordered_map<NodeType, NodeId> node_to_idx;
    int num_nodes;
};

Graph<std::string> parse_graph(std::string_view content)
{
    int node_idx = 0;
    std::unordered_map<std::string, int> node_to_idx;
    std::unordered_map<std::string, std::vector<std::string>> adj_list_str;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        // Parse format: "node: neighbor1 neighbor2 ..."
        auto colon_pos = line.find(':');
        if (colon_pos == std::string_view::npos)
        {
            continue;
        }

        auto node_name = std::string(line.substr(0, colon_pos));
        auto rest = line.substr(colon_pos + 2);  // Skip ": "

        if (node_to_idx.find(node_name) == node_to_idx.end())
        {
            node_to_idx[node_name] = node_idx++;
        }

        // Parse neighbors
        for (auto neighbor_rng : split(rest, ' '))
        {
            auto neighbor_name = std::string(to_string_view(neighbor_rng));
            if (neighbor_name.empty())
            {
                continue;
            }

            if (node_to_idx.find(neighbor_name) == node_to_idx.end())
            {
                node_to_idx[neighbor_name] = node_idx++;
            }

            adj_list_str[node_name].push_back(neighbor_name);
        }
    }

    // Build integer adjacency list
    std::vector<std::vector<int>> adj_list(node_idx + 1);
    for (const auto& [node_name, neighbor_names] : adj_list_str)
    {
        for (const auto& neighbor_name : neighbor_names)
        {
            adj_list[node_to_idx[node_name]].push_back(node_to_idx[neighbor_name]);
        }
    }

    return Graph<std::string>{std::move(adj_list), std::move(node_to_idx), node_idx};
}

// DFS with memoization to count paths in DAG, with cycle detection
template <GraphNode NodeType>
long long dfs(const Graph<NodeType>& graph, std::vector<long long>& memo, typename Graph<NodeType>::NodeId start,
              typename Graph<NodeType>::NodeId end)
{
    if (start == end)
    {
        return 1;
    }

    if (memo[start] != -1)
    {
        if (memo[start] == -2)
        {
            // Cycle detected, invalid for DAG
            return 0;
        }
        return memo[start];
    }

    memo[start] = -2;  // Mark as visiting
    long long total_paths = 0;
    for (int neighbor : graph.adj_list[start])
    {
        total_paths += dfs(graph, memo, neighbor, end);
    }
    memo[start] = total_paths;
    return total_paths;
}

long long part1(const Graph<std::string>& graph)
{
    std::vector<long long> memo(graph.num_nodes + 1, -1);
    return dfs(graph, memo, graph.node_to_idx.at("you"), graph.node_to_idx.at("out"));
}

long long part2(const Graph<std::string>& graph)
{
    const std::vector<std::vector<std::pair<std::string, std::string>>> test_paths = {
        // Option 1
        {{"svr", "fft"}, {"fft", "dac"}, {"dac", "out"}},
        // Option 2
        {{"svr", "dac"}, {"dac", "fft"}, {"fft", "out"}}};

    long long total_paths = 0;

    for (const auto& test_path : test_paths)
    {
        long long path_for_this_option = 1;
        std::vector<long long> memo(graph.num_nodes + 1, -1);

        for (const auto& [start_name, end_name] : test_path)
        {
            int start_idx = graph.node_to_idx.at(start_name);
            int end_idx = graph.node_to_idx.at(end_name);

            path_for_this_option *= dfs(graph, memo, start_idx, end_idx);
            memo.assign(graph.num_nodes + 1, -1);
        }

        total_paths += path_for_this_option;
    }

    return total_paths;
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day11/long.txt");
        const auto graph = parse_graph(content);

        assert(part1(graph) == 607);
        assert(part2(graph) == 506264456238938);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
