def read_output_file(filename):
    with open(filename, 'r') as f:
        nr_backbone = int(f.readline())
        backbone_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_backbone)]
        nr_router = int(f.readline())
        router_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_router)]
    return backbone_coords, router_coords

def any_wall_between(char_map, point_a, point_b):
    for i in range(point_a[0], point_b[0] + 1):
        for j in range(point_a[1], point_b[1] + 1):
            if char_map[i][j] == '#':
                return True
    return False

def count_cells_covered(router_coords, char_map, router_radius):
    cells_covered = 0
    previously_covered = set()

    for x, y in router_coords:
        for i in range(x - router_radius, x + router_radius + 1):
            for j in range(y - router_radius, y + router_radius + 1):
                upper_left = (min(x, i), min(y, j))
                bottom_right = (max(x, i), max(y, j))
                if char_map[i][j] == '.' and not any_wall_between(char_map, upper_left, bottom_right):
                    if (i, j) not in previously_covered:
                        previously_covered.add((i, j))
                        cells_covered += 1
    return cells_covered

solutions = ["sol1", "sol2", "sol2i", "sol3"]
scores = {key: {} for key in solutions}
input_files = ["charleston_road", "lets_go_higher", "opera", "rue_de_londres"]

for input_file in input_files:
    with open(f"input_files/{input_file}.in", 'r') as f:
        nr_rows, nr_columns, router_radius = list(map(int, f.readline().split()))
        backbone_cost, router_cost, budget = list(map(int, f.readline().split()))
        backbone_x, backbone_y = list(map(int, f.readline().split()))
        char_map = [list(line.rstrip('\n')) for line in f if line.strip()]

        for solution in solutions:
            output_file = f"output_files/{solution}/{input_file}"
            try:
                backbone_coords, router_coords = read_output_file(output_file)
                # Check that it fits in the budget
                budget_used = len(backbone_coords) * backbone_cost + len(router_coords) * router_cost
                assert(budget_used <= budget)

                # Check that no router is inside of a wall/out of bounds, and on a backbone
                for x, y in router_coords:
                    assert(char_map[x][y] != '#' and char_map[x][y] != '-')
                    assert((x, y) in backbone_coords)
                
                # placing another backbone on the original backbone cell is redundant
                assert((backbone_x, backbone_y) not in backbone_coords)

                # Get the number of cells covered by a router
                cells_covered = count_cells_covered(router_coords, char_map, router_radius)
                score = cells_covered * 1000 + budget - budget_used

                scores[solution][input_file] = (score, cells_covered)

            except FileNotFoundError:
                print(f"Error: File '{output_file}' not found.")


for solution, input_to_score in scores.items():
    print(f"For {solution}:")

    final_score = total_covered = 0
    for input_file, score_and_cov in input_to_score.items():
        print(f"--->{input_file}: {score_and_cov[0]:,} score, {score_and_cov[1]:,} cells covered.")
        final_score += score_and_cov[0]
        total_covered += score_and_cov[1]
    print(f"Final score: {final_score:,}, cells covered: {total_covered:,}\n")