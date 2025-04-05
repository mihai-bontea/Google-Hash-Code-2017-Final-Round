import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter

def read_char_map(filename):
    with open(filename, 'r') as f:
        char_map = [list(line.rstrip('\n')) for line in f if line.strip()]
    return char_map

char_map = read_char_map("lets_go_higher.in")

char_to_code = {
    '#': 0,  # wall
    '.': 1,  # empty
    '-': 2,  # useless
}

# Converting char map to numeric code map
numeric_map = np.array([[char_to_code[ch] for ch in row] for row in char_map])

from matplotlib.colors import ListedColormap
cmap = ListedColormap(['#092327', '#00a9a5', '#0b5351'])
# cmap = ListedColormap(['#11151c', '#364156', '#212d40'])
# cmap = ListedColormap(['#084b83', '#bbe6e4', '#42bfdd'])
# cmap = ListedColormap(['#084b83', '#bbe6e4', '#212d40'])
# cmap = ListedColormap(['#092327', '#bbe6e4', '#0b5351'])

plt.figure(figsize=(40, 40))
# plt.imshow(numeric_map, cmap=cmap)
# plt.imshow(numeric_map, cmap=cmap, interpolation='nearest')  # blocky
plt.imshow(numeric_map, cmap=cmap, interpolation='bilinear')  # smoother
plt.axis('off')
plt.show()