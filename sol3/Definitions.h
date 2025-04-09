#pragma once
/*
 * Helper functions related to points and rectangles
 */

#include <iostream>

using Point = std::pair<int, int>;
using Rectangle = std::pair<Point, Point>;

inline unsigned int compute_rect_sum(const Rectangle& rectangle, const std::unique_ptr<std::unique_ptr<unsigned int[]>[]>& int_map)
{
    // rectangle [(0, 0), (i2, j2)]
    unsigned int result = int_map[rectangle.second.first][rectangle.second.second];

    // subtracting [(0, 0), (i1 - 1, j2)], which is the rectangle above		(1)
    const int i1 = std::max(((int)rectangle.first.first - 1), 0);
    result -= int_map[i1][rectangle.second.second];

    // subtracting [(0, 0), (i2, j1 - 1)]									(2)
    const int j1 = std::max(((int)rectangle.first.second - 1), 0);
    result -= int_map[rectangle.second.first][j1];

    // Adding the intersection between (1) and (2) which has been subtracted twice
    //[(0, 0), (i1 - 1, j1 - 1)]
    return result + int_map[i1][j1];
};

inline bool is_valid(const Point& point, const int nr_rows, const int nr_columns)
{
    return point.first < nr_rows && point.second < nr_columns &&
           point.first >= 0 && point.second >= 0;
}

inline unsigned int get_distance(const Point& a, const Point& b)
{
    const int x_dist = abs((int)a.first - (int)b.first);
    const int y_dist = abs((int)a.second - (int)b.second);

    return std::max(x_dist, y_dist);
}