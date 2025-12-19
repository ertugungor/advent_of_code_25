from functools import lru_cache


def read_input(filename):
    with open(filename) as f:
        return [line.strip() for line in f if line.strip()]


def solve(lines, max_slots):
    """Solve knapsack problem for given number of slots."""
    total = 0
    for line in lines:

        @lru_cache(maxsize=None)
        def digit_knapsack(idx, num_slots):
            if idx >= len(line) or num_slots <= 0:
                return 0

            digit = int(line[idx])
            power = min(num_slots - 1, len(line) - idx - 1)
            val = (10**power) * digit

            include = val + digit_knapsack(idx + 1, num_slots - 1)
            exclude = digit_knapsack(idx + 1, num_slots)

            return max(include, exclude)

        total += digit_knapsack(0, max_slots)
        digit_knapsack.cache_clear()

    return total


def part1(lines):
    return solve(lines, max_slots=2)


def part2(lines):
    return solve(lines, max_slots=12)


if __name__ == "__main__":
    lines = read_input("long.txt")

    assert part1(lines) == 17435
    assert part2(lines) == 172886048065379
