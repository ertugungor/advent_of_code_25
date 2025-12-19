#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "puzzles/common.hpp"

namespace aoc
{

template <typename T>
concept MatrixElement = std::floating_point<T> || std::integral<T>;

template <typename T>
concept ButtonStructure = requires(T buttons, std::size_t i) {
    { buttons.size() } -> std::convertible_to<std::size_t>;
    { buttons[i] } -> std::ranges::range;
    { buttons[i][0] } -> std::convertible_to<int>;
};

template <typename T>
concept TargetVector = requires(T targets, std::size_t i) {
    { targets.size() } -> std::convertible_to<std::size_t>;
    { targets[i] } -> std::convertible_to<int>;
};

/**
 * @brief Solves linear systems for button-press optimization.
 *
 * Constructs A*x=b where:
 * - A[i][j] = 1 if button j affects counter i, else 0
 * - b[i] = targets[i]
 * - x[j] = times to press button j (non-negative integer)
 *
 * Algorithm:
 * 1. Construct augmented matrix [A|b]
 * 2. Gaussian elimination to RREF
 * 3. Identify free variables
 * 4. Recursive search over free variable values
 * 5. Return min button presses or nullopt
 *
 * Complexity: O((m^2)n + V^k) where m=equations, n=vars, V=max_target,
 * k=free_vars
 */

template <MatrixElement T = double>
class GaussianSolver
{
  public:
    using Matrix = std::vector<std::vector<T>>;
    using Solution = std::optional<std::vector<long long>>;

    static constexpr T kEpsilon = T{1e-9};

    template <TargetVector Targets, ButtonStructure Buttons>
    Solution solve(const Targets& targets, const Buttons& buttons);

  private:
    void reduce_to_rref(Matrix& matrix, std::vector<int>& col_to_pivot_row);
    Solution find_min_solution(const Matrix& matrix, const std::vector<int>& col_to_pivot_row,
                               const std::vector<int>& targets, const std::vector<std::vector<int>>& buttons) const;
    void identify_variables(const std::vector<int>& col_to_pivot_row, std::vector<int>& free_vars,
                            std::vector<int>& basic_vars) const;
    Solution compute_fixed_solution(const Matrix& matrix, const std::vector<int>& col_to_pivot_row) const;
};

// Parse buttons from format: "(1,2,3) (4,5) (6)"
std::vector<std::vector<int>> parse_buttons(std::string_view buttons_str)
{
    std::vector<std::vector<int>> buttons;

    // Use istringstream for complex parsing (similar to day06 reasoning)
    std::string str{buttons_str};
    std::istringstream ss{str};
    std::string token;

    while (ss >> token)
    {
        if (token.empty() || token[0] != '(')
        {
            continue;
        }

        // Remove parentheses
        std::string inner = token.substr(1, token.size() - 2);
        std::vector<int> button_indices;

        // Parse comma-separated integers
        for (auto part : split(inner, ','))
        {
            button_indices.push_back(to_int<int>(to_string_view(part)));
        }

        buttons.push_back(button_indices);
    }

    return buttons;
}

void dp(const std::vector<int>& target_lights, const std::vector<std::vector<int>>& buttons, size_t index,
        std::vector<int>& current_lights, int num_press, int& min_press)
{
    if (index == buttons.size())
    {
        if (current_lights == target_lights)
        {
            min_press = std::min(min_press, num_press);
        }
        return;
    }

    // Press the button
    for (int btn_index : buttons[index])
    {
        current_lights[btn_index] = 1 - current_lights[btn_index];
    }
    dp(target_lights, buttons, index + 1, current_lights, num_press + 1, min_press);

    // Undo
    for (int btn_index : buttons[index])
    {
        current_lights[btn_index] = 1 - current_lights[btn_index];
    }

    // Skip
    dp(target_lights, buttons, index + 1, current_lights, num_press, min_press);
}

long long part1(std::string_view content)
{
    long long total_min_press = 0;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        // Parse format: "[.#.#] (1,2) (3,4) ..."
        auto first_button_pos = line.find('(');
        if (first_button_pos == std::string_view::npos)
        {
            continue;
        }

        auto lights_str = line.substr(0, first_button_pos - 1);
        auto buttons_str = line.substr(first_button_pos);

        // Parse target lights
        std::vector<int> target_lights;
        for (char c : lights_str)
        {
            if (c == '.')
            {
                target_lights.push_back(0);
            }
            else if (c == '#')
            {
                target_lights.push_back(1);
            }
        }

        auto buttons = parse_buttons(buttons_str);

        int min_press = std::numeric_limits<int>::max();
        std::vector<int> current_lights(target_lights.size(), 0);
        dp(target_lights, buttons, 0, current_lights, 0, min_press);

        total_min_press += min_press;
    }

    return total_min_press;
}

template <MatrixElement T>
void GaussianSolver<T>::identify_variables(const std::vector<int>& col_to_pivot_row, std::vector<int>& free_vars,
                                           std::vector<int>& basic_vars) const
{
    const int num_vars = static_cast<int>(col_to_pivot_row.size());
    for (int j = 0; j < num_vars; ++j)
    {
        if (col_to_pivot_row[j] == -1)
        {
            free_vars.push_back(j);
        }
        else
        {
            basic_vars.push_back(j);
        }
    }
}

template <MatrixElement T>
auto GaussianSolver<T>::find_min_solution(const Matrix& matrix, const std::vector<int>& col_to_pivot_row,
                                          const std::vector<int>& targets,
                                          const std::vector<std::vector<int>>& buttons) const -> Solution
{
    std::vector<int> free_vars;
    std::vector<int> basic_vars;
    identify_variables(col_to_pivot_row, free_vars, basic_vars);

    if (free_vars.empty())
    {
        return compute_fixed_solution(matrix, col_to_pivot_row);
    }

    long long min_total_presses = std::numeric_limits<long long>::max();
    Solution best_solution = std::nullopt;
    std::vector<int> current_free_vals(free_vars.size());

    const auto is_negative_or_non_integer = [&](T val) {
        if (val < -kEpsilon)
        {
            return true;
        }
        const long long int_val = std::llround(val);
        return std::abs(val - static_cast<T>(int_val)) > kEpsilon;
    };

    // Base case lambda
    auto base_case = [&]() -> void {
        std::vector<long long> candidate(free_vars.size() + basic_vars.size(), 0LL);
        long long current_sum = 0;
        for (int i = 0; i < static_cast<int>(free_vars.size()); ++i)
        {
            candidate[free_vars[i]] = current_free_vals[i];
            current_sum += current_free_vals[i];
        }

        bool solution_possible = true;
        for (int basic_idx : basic_vars)
        {
            const int row_idx = col_to_pivot_row[basic_idx];
            T target_val = matrix[row_idx][free_vars.size() + basic_vars.size()];

            for (int i = 0; i < static_cast<int>(free_vars.size()); ++i)
            {
                target_val -= matrix[row_idx][free_vars[i]] * current_free_vals[i];
            }

            if (is_negative_or_non_integer(target_val))
            {
                solution_possible = false;
                break;
            }
            const long long rounded = std::llround(target_val);
            candidate[basic_idx] = rounded;
            current_sum += rounded;
        }

        if (solution_possible && current_sum < min_total_presses)
        {
            min_total_presses = current_sum;
            best_solution = candidate;
        }
    };

    // Recursive lambda for backtracking
    auto recursive_search = [&](this auto&& self, int free_idx) -> void {
        const int num_eqs = static_cast<int>(targets.size());
        if (free_idx == static_cast<int>(free_vars.size()))
        {
            base_case();
            return;
        }

        const int free_var_idx = free_vars[free_idx];
        int limit = *std::max_element(targets.begin(), targets.end()) + 1;

        for (int i = 0; i < num_eqs; ++i)
        {
            bool affects = std::ranges::find(buttons[free_var_idx], i) != buttons[free_var_idx].end();
            if (affects)
            {
                limit = std::min(limit, targets[i]);
            }
        }

        for (int val = 0; val <= limit; ++val)
        {
            current_free_vals[free_idx] = val;
            bool partial_valid = true;

            for (int basic_idx : basic_vars)
            {
                const int row_idx = col_to_pivot_row[basic_idx];
                T target = matrix[row_idx][free_vars.size() + basic_vars.size()];
                bool depends_on_unassigned_free_vars = false;

                for (int i = 0; i < static_cast<int>(free_vars.size()); ++i)
                {
                    const T coeff = matrix[row_idx][free_vars[i]];
                    if (i <= free_idx)
                    {
                        target -= coeff * current_free_vals[i];
                    }
                    else if (std::abs(coeff) > kEpsilon)
                    {
                        depends_on_unassigned_free_vars = true;
                    }
                }

                if (!depends_on_unassigned_free_vars)
                {
                    if (is_negative_or_non_integer(target))
                    {
                        partial_valid = false;
                        break;
                    }
                }
                else if (target < -kEpsilon)
                {
                    bool can_increase = false;
                    for (int future_free_idx = free_idx + 1; future_free_idx < static_cast<int>(free_vars.size());
                         ++future_free_idx)
                    {
                        if (matrix[row_idx][free_vars[future_free_idx]] < -kEpsilon)
                        {
                            can_increase = true;
                            break;
                        }
                    }
                    if (!can_increase)
                    {
                        partial_valid = false;
                        break;
                    }
                }
            }

            if (partial_valid)
            {
                self(free_idx + 1);
            }
        }
    };

    recursive_search(0);

    return best_solution;
}

template <MatrixElement T>
template <TargetVector Targets, ButtonStructure Buttons>
auto GaussianSolver<T>::solve(const Targets& targets, const Buttons& buttons) -> Solution
{
    const int num_eqs = static_cast<int>(targets.size());
    const int num_vars = static_cast<int>(buttons.size());

    Matrix matrix(num_eqs, std::vector<T>(num_vars + 1, T{}));

    for (int j = 0; j < num_vars; ++j)
    {
        for (int i : buttons[j])
        {
            if (i < num_eqs)
            {
                matrix[i][j] = T{1};
            }
        }
    }

    for (int i = 0; i < num_eqs; ++i)
    {
        matrix[i][num_vars] = static_cast<T>(targets[i]);
    }

    std::vector<int> col_to_pivot_row(num_vars, -1);
    reduce_to_rref(matrix, col_to_pivot_row);

    for (const auto& row : matrix)
    {
        bool all_zero = true;
        for (int j = 0; j < num_vars; ++j)
        {
            if (std::abs(row[j]) > kEpsilon)
            {
                all_zero = false;
                break;
            }
        }
        if (all_zero && std::abs(row[num_vars]) > kEpsilon)
        {
            return std::nullopt;
        }
    }

    return find_min_solution(matrix, col_to_pivot_row, targets, buttons);
}

template <MatrixElement T>
void GaussianSolver<T>::reduce_to_rref(Matrix& matrix, std::vector<int>& col_to_pivot_row)
{
    const int num_eqs = static_cast<int>(matrix.size());
    const int num_vars = static_cast<int>(col_to_pivot_row.size());

    int pivot_row = 0;

    for (int col_idx = 0; col_idx < num_vars && pivot_row < num_eqs; ++col_idx)
    {
        int sel = -1;
        for (int row_idx = pivot_row; row_idx < num_eqs; ++row_idx)
        {
            if (std::abs(matrix[row_idx][col_idx]) > kEpsilon)
            {
                sel = row_idx;
                break;
            }
        }

        if (sel == -1)
        {
            continue;
        }

        std::swap(matrix[pivot_row], matrix[sel]);

        const T pivot_val = matrix[pivot_row][col_idx];
        for (int j = col_idx; j <= num_vars; ++j)
        {
            matrix[pivot_row][j] /= pivot_val;
        }

        for (int row_idx = 0; row_idx < num_eqs; ++row_idx)
        {
            if (row_idx == pivot_row)
            {
                continue;
            }
            const T factor = matrix[row_idx][col_idx];
            if (std::abs(factor) > kEpsilon)
            {
                for (int j = col_idx; j <= num_vars; ++j)
                {
                    matrix[row_idx][j] -= factor * matrix[pivot_row][j];
                }
            }
        }

        col_to_pivot_row[col_idx] = pivot_row;
        ++pivot_row;
    }
}

template <MatrixElement T>
auto GaussianSolver<T>::compute_fixed_solution(const Matrix& matrix,
                                               const std::vector<int>& col_to_pivot_row) const -> Solution
{
    const int num_vars = static_cast<int>(col_to_pivot_row.size());
    std::vector<long long> solution(num_vars, 0LL);
    long long current_presses = 0;
    for (int j = 0; j < num_vars; ++j)
    {
        const int row = col_to_pivot_row[j];
        const T val = matrix[row][num_vars];
        if (val < -kEpsilon || std::abs(val - static_cast<T>(std::llround(val))) > kEpsilon)
        {
            return std::nullopt;
        }
        const long long rounded = std::llround(val);
        solution[j] = rounded;
        current_presses += rounded;
    }
    if (current_presses < std::numeric_limits<long long>::max())
    {
        return solution;
    }
    return std::nullopt;
}

long long part2(std::string_view content)
{
    GaussianSolver<double> solver;
    std::vector<long long> line_totals;

    for (auto line_rng : get_lines(content))
    {
        auto line = to_string_view(line_rng);
        if (line.empty())
        {
            continue;
        }

        // Parse format: "(1,2) (3,4) ... {5,6,7}"
        auto first_button_pos = line.find('(');
        auto first_joltage_pos = line.find('{');

        if (first_button_pos == std::string_view::npos || first_joltage_pos == std::string_view::npos)
        {
            continue;
        }

        auto buttons_str = line.substr(first_button_pos, first_joltage_pos - first_button_pos - 1);
        auto joltage_str = line.substr(first_joltage_pos + 1);

        // Remove braces from joltage string
        joltage_str = joltage_str.substr(0, joltage_str.size() - 1);

        // Parse target joltages
        std::vector<int> target_joltages;
        for (auto part : split(joltage_str, ','))
        {
            target_joltages.push_back(to_int<int>(to_string_view(part)));
        }

        auto buttons = parse_buttons(buttons_str);

        if (auto solution = solver.solve(target_joltages, buttons))
        {
            line_totals.push_back(std::accumulate(solution->begin(), solution->end(), 0LL));
        }
    }

    return std::accumulate(line_totals.begin(), line_totals.end(), 0LL);
}

}  // namespace aoc

int main()
{
    try
    {
        using namespace aoc;
        const auto content = read_file("puzzles/day10/long.txt");

        assert(part1(content) == 505);
        assert(part2(content) == 20002);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
