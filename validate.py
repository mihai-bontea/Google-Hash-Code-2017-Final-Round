char_to_code = {
    '#': 0,  # wall
    '.': 1,  # empty
    '-': 2,  # useless
}

def read_output_file(filename):
    with open(filename, 'r') as f:
        nr_backbone = int(f.readline())
        backbone_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_backbone)]
        nr_router = int(f.readline())
        router_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_router)]
    return backbone_coords, router_coords

solutions = ["sol1", "sol2", "sol2i"]
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
                assert(len(backbone_coords) * backbone_cost + len(router_coords) * router_cost <= budget)

                # Check that no router is inside of a wall/out of bounds
                for x, y in router_coords:
                    assert(char_map[x][y] != 0 and char_map[x][y] != 2)

            except FileNotFoundError:
                print(f"Error: File '{output_file}' not found.")
