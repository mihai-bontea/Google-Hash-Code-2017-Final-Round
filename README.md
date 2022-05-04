# Google Hash Code 2017 Final Round

## Solution 1

### Representing the data

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


* charleston_road.in: Cells covered = 9117, Score = 9117063

* lets_go_higher.in: Cells covered = 46618, Score = 46618042

* opera.in: Cells covered = 11240, Score = 11240038

* rue_de_londres.in: Cells covered = 5565, Score = 5565044

* **Final score**: 72540187

This solution would have gotten place 48 in the competition. It is pretty flawed due to the fact that as multiple routers are added, this is not reflected in the 2D Segment Tree, which leads to a lot of overlaps with other routers. One fix attempted was to make the 2D Segment Tree hold the first 5 or 10 maximums so that we have more positions to choose from, but this comes at a great memory cost, and the execution time for the large dataset goes from 10 minutes to well over 90 minutes.