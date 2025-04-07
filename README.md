# Google Hash Code 2017 Final Round (Router Placement)

<p align="center">
  <img src="https://github.com/user-attachments/assets/46f03c19-ac45-43db-af36-176572b49157" alt="Image" />
</p>

>Who doesn't love wireless Internet? Millions of people rely on it for productivity and fun in countless cafes, railway stations and public areas of all sorts. For many institutions, ensuring wireless Internet access is now almost as important a feature of building facilities as the access to water and electricity.

>Typically, buildings are connected to the Internet using a fiber backbone. In order to provide wireless Internet access, wireless routers are placed around the building and connected using fiber cables to the backbone. The larger and more complex the building, the harder it is to pick router locations and decide how to lay down the connecting cables.

>**Given a building plan, decide where to put wireless routers and how to connect them to the fiber backbone to maximize coverage and minimize cost.**

## Solution 1

### Representing the data

![Image](https://github.com/user-attachments/assets/bf0b6e9d-055c-4a38-9f9d-a18d715fb25f)

A router can cover a square from the map if there are no walls in the smallest rectangle which contains both the router and the square. Since this will be a common query, an efficient way
to get the number of walls in any given rectangle is needed.

For this purpose I used **dynamic programming**, partial sums more specifically. In this case, **nr_walls[i][j]** will hold the number of walls in the rectangle **[(1, 1), (i, j)]**. 
So for any rectangle **[(i1, j1), (i2, j2)]** the number of walls can be calculated as **[(1, 1), (i2, j2)]** - **[(1, 1), (i1 - 1, j2)]** - **[(1, 1), (i2, j1 - 1)]** + **[(1, 1), (i1 - 1, j1 - 1)]**
(subtracting the rectangle above, subtracting the rectangle to the left, and adding the rectangle that's formed by the overlapping of the two previously mentioned, since it is subtracted twice).

Now that we have an **O(1)** way to determine whether a block is going to be covered by the router, we can determine the coverage a router would obtain if it were placed at any position **(i, j)**.
This would mean a complexity of **O(N * M * R)** where **R** is the radius of the router. However, this task is **easily done in parallel**. For this purpose, I created 12 threads, each computing
the coverage for 1/12 of the map.

Another query which will be done a lot is, "which position for the router will yield the biggest coverage in my general area?". This can be done by iterating a certain rectangle of the matrix
computed above, and determining its maximum. However, this scales badly with many queries. For this reason, a **2D Segment Tree** is created, which gets the maximum of any rectangle **[(i1, j1), (i2, j2)]**
in **O(log M * N)**.

### Strategy

Go over the routers already placed(initially, only the starting cell of the backbone), and get the best position for a router in the general area(different area sizes are tried). A list of the cells covered
by a new would-be router is obtained, and the amount of overlap with other routers is calculated, and it is given a score, based on the following formula: **actual_coverage * 0.60 + ((200 - distance) / 2 * 0.40)**,
where **actual_coverage** is the number of cells this new router covers, minus the overlap with other routers. The best scoring router is added to the final result, and this process is repeated while there
are funds left.

### Scoring

| File Name          | Score    | Cells Covered |
|--------------------|---------:|--------------:|
| charleston_road    | 9117063  |         9117  |
| lets_go_higher     | 46618042 |        46618  |
| opera              | 11240038 |        11240  |
| rue_de_londres.out | 5565044  |         5565  |
| **Final**          | 72540187 |       72540   |

This solution would have gotten place 48 in the competition. It is pretty flawed due to the fact that as multiple routers are added, this is not reflected in the 2D Segment Tree, which leads to a lot of overlaps with other routers. One fix attempted was to make the 2D Segment Tree hold the first 5 or 10 maximums so that we have more positions to choose from, but this comes at a great memory cost, and the execution time for the large dataset goes from 10 minutes to well over 90 minutes.

## Solution 2

### Strategy

The building map is iterated, and whenever space allows it, a square of l = **2 * R + 1** is created and stored. This square can be fully covered by a router placed in the middle. From this square, using Lee's algorithm, the rest of the map is 'filled', which means trying to create new adjacent squares(to N,S,E,W). Those squares will make up a **component**, where routers are placed in the middle of each square, and each router is connected by cable to its parent router(the one that spawned it in the Lee's algorithm). This means that within a component **there is absolutely no overlap**. 

### Scoring

| File Name          | Score    | Cells Covered |
|--------------------|---------:|--------------:|
| charleston_road    | 9729041  |         9702  |
| lets_go_higher     | 191416954|       189123  |
| opera              | 112746861|       112725  |
| rue_de_londres.out | 18975262 |        18963  |
| **Final**          | 332868118|      330513   |

Apparently, there are not enough open areas to fully use the available funds, so this solution, while better scoring than the first, misses out on many potential cells to cover.

## Solution 2 Improved

### Strategy

The iterations of Lee's algorithm are no longer initiated through a top-down, left-right traversal of the map. Instead, it is done through a spiral traversal, starting from the original fiber backbone position. This ensures a better 'locality' of the routers.

In addition, a **k-d tree** is used to always pick the closest router/backbone to connect a new router to. This way, the least amount of cable is alway used.

### Scoring

| File Name          | Score    | Cells Covered |
|--------------------|---------:|--------------:|
| charleston_road    | 9729218  |         9702  |
| lets_go_higher     | 191050052|       188639  |
| opera              | 117484399|       117450  |
| rue_de_londres.out | 20742355 |        20727  |
| **Final**          | 339006024|      336518   |

## Visualizer

The visualizer script reads the input and output files, and creates a visual representation of the solutions. The routers are represented by red, and
the cables are represented by yellow. The validator script checks that the solutions are within budget.

![Image](https://github.com/user-attachments/assets/b6421748-1ec8-4cd0-9ded-21fcddac1757)