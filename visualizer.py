import numpy as np
import matplotlib.pyplot as plt
from copy import deepcopy
from matplotlib.colors import ListedColormap

char_to_code = {
    '#': 0,  # wall
    '.': 1,  # empty
    '-': 2,  # useless
}

def read_char_map(filename):
    with open(filename, 'r') as f:
        for _ in range(3): next(f)
        char_map = [list(line.rstrip('\n')) for line in f if line.strip()]
    return char_map

def read_output_file(filename):
    with open(filename, 'r') as f:
        nr_backbone = int(f.readline())
        backbone_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_backbone)]
        nr_router = int(f.readline())
        router_coords = [tuple(map(int, f.readline().split())) for _ in range(nr_router)]
    return backbone_coords, router_coords

def generate_picture(numeric_map, filename):
    cmap = ListedColormap(['#092327', '#00a9a5', '#0b5351', '#ffd166', '#ff0000'])
    plt.figure(figsize=(40, 40))
    plt.imshow(numeric_map, cmap=cmap, interpolation='bilinear')
    plt.axis('off')
    plt.savefig(filename, bbox_inches='tight', pad_inches=0, dpi=300)

solutions = ["sol1", "sol2"]
input_files = ["charleston_road", "lets_go_higher", "opera", "rue_de_londres"]

for input_file in input_files:
    char_map = read_char_map(f"input_files/{input_file}.in")
    numeric_map = np.array([[char_to_code[ch] for ch in row] for row in char_map])

    for solution in solutions:
        output_file = f"output_files/{solution}/{input_file}"
        try:
            backbone_coords, router_coords = read_output_file(output_file)
            numeric_map_copy = deepcopy(numeric_map)
            for x, y in backbone_coords: numeric_map_copy[x][y] = 3
            for x, y in router_coords: numeric_map_copy[x][y] = 4

            generate_picture(numeric_map_copy, f"visualizers/{solution}/{input_file}.png")
        except FileNotFoundError:
            print(f"Error: File '{output_file}' not found.")

