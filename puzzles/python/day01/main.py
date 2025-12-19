def read_input(filename):
    with open(filename) as f:
        return [line.strip() for line in f if line.strip()]


def part1(lines):
    start = 50
    count = 0
    for line in lines:
        rot = line[0]
        val = int(line[1:])

        start = (start - val if rot == "L" else start + val) % 100
        if start == 0:
            count += 1

    return count


def part2(lines):
    start = 50
    count = 0

    for line in lines:
        rot = line[0]
        val = int(line[1:])

        if rot == "L":
            before = (start - 1) // 100
            after = (start - 1 - val) // 100
            count += before - after
            start = (start - val) % 100
        else:
            before = (start - 1) // 100
            after = (start - 1 + val) // 100
            count += after - before
            start = (start + val) % 100

    return count


if __name__ == "__main__":
    lines = read_input("long.txt")

    assert part1(lines) == 1154
    assert part2(lines) == 6819
