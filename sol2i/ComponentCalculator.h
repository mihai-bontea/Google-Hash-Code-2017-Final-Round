#pragma once
#include <unordered_set>
#include <algorithm>
#include "Data.h"
#include "Definitions.h"
#include <queue>

class ComponentCalculator
{
private:
    const Data& data;
    unique_ptr<unique_ptr<unsigned int[]>[]> nr_coverable_cells;
    unique_ptr<unique_ptr<bool[]>[]> visited;

    bool can_place_router(unsigned int i, unsigned int j)
    {
        const auto compute_rect_sum = [&](Matrix matrix)
        {
            // rectangle [(0, 0), (i2, j2)]
            unsigned int result = nr_coverable_cells[matrix.second.first][matrix.second.second];

            // subtracting [(0, 0), (i1 - 1, j2)], which is the rectangle above		(1)
            const int i1 = max(((int)matrix.first.first - 1), 0);
            result -= nr_coverable_cells[i1][matrix.second.second];

            // subtracting [(0, 0), (i2, j1 - 1)]									(2)
            const int j1 = max(((int)matrix.first.second - 1), 0);
            result -= nr_coverable_cells[matrix.second.first][j1];

            // Adding the intersection between (1) and (2) which has been subtracted twice
            //[(0, 0), (i1 - 1, j1 - 1)]
            return result + nr_coverable_cells[i1][j1];
        };

        const int upper_left_i = i - data.router_radius;
        const int upper_left_j = j - data.router_radius;

        const unsigned int bottom_right_i = i + data.router_radius;
        const unsigned int bottom_right_j = j + data.router_radius;

        // bound-checking
        if (upper_left_i < 0 || upper_left_j < 0 || bottom_right_i >= data.nr_rows || bottom_right_j >= data.nr_columns)
            return false;

        // checking if all cells are unvisited
        for (unsigned int i = upper_left_i; i <= bottom_right_i; ++i)
            for (unsigned int j = upper_left_j; j <= bottom_right_j; ++j)
                if (visited[i][j])
                    return false;

        const Point upper_left = make_pair(upper_left_i, upper_left_j);
        const Point bottom_right = make_pair(bottom_right_i, bottom_right_j);
        const auto nr_coverable_cells_in_matrix = compute_rect_sum(make_pair(upper_left, bottom_right));
        return nr_coverable_cells_in_matrix == (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1);
    };

    vector<Point> apply_Lee(Point initial_router)
    {
        auto are_coords_valid = [&](const int i, const int j)
        {
            return (i >= 0 && j >= 0 && i < data.nr_rows&& j < data.nr_columns);
        };

        auto add_as_visited = [&](Point router_position)
        {
            const int upper_left_i = router_position.first - data.router_radius;
            const int upper_left_j = router_position.second - data.router_radius;

            const unsigned int bottom_right_i = router_position.first + data.router_radius;
            const unsigned int bottom_right_j = router_position.second + data.router_radius;

            for (unsigned int i = upper_left_i; i <= bottom_right_i; ++i)
                for (unsigned int j = upper_left_j; j <= bottom_right_j; ++j)
                    visited[i][j] = true;
        };

        const int di[4] = { 0, 0, 1, -1 };
        const int dj[4] = { 1, -1, 0, 0 };
        const int distance_multiplier = data.router_radius * 2 + 1;

        vector<Point> result;
        result.push_back(initial_router);
        add_as_visited(initial_router);

        queue<Point> routers;
        routers.push(initial_router);

        while (!routers.empty())
        {
            auto current_router = routers.front();
            routers.pop();

            for (unsigned int direction = 0; direction < 4; ++direction)
            {
                const int new_i = (int)current_router.first + distance_multiplier * di[direction];
                const int new_j = (int)current_router.second + distance_multiplier * dj[direction];
                if (are_coords_valid(new_i, new_j) && can_place_router(new_i, new_j) &&
                    data.building_plan[new_i][new_j] == '.')
                {
                    const Point new_router = make_pair(new_i, new_j);
                    add_as_visited(new_router);
                    routers.push(new_router);
//                    routers_to_parents.insert(make_pair(new_router, current_router));
                    result.push_back(new_router);
                }
            }
        }

        return result;
    }

public:

    ComponentCalculator(const Data& data): data{data}
    {
        // Allocating memory
        nr_coverable_cells = make_unique<unique_ptr<unsigned int[]>[]>(data.nr_rows);
        for (unsigned int index = 0; index < data.nr_rows; ++index)
            nr_coverable_cells[index] = make_unique<unsigned int[]>(data.nr_columns);

        // Determining the nr of cells which can be covered in any rectangle
        for (unsigned int i = 0; i < data.nr_rows; ++i)
            nr_coverable_cells[i][0] = (data.building_plan[i][0] == '.');

        // First, determining line sum
        for (unsigned int i = 0; i < data.nr_rows; ++i)
            for (unsigned int j = 1; j < data.nr_columns; ++j)
                nr_coverable_cells[i][j] = nr_coverable_cells[i][j - 1] + (data.building_plan[i][j] == '.');

        // Second, determining rectangle sum
        for (unsigned int j = 0; j < data.nr_columns; ++j)
            for (unsigned int i = 1; i < data.nr_rows; ++i)
                nr_coverable_cells[i][j] += nr_coverable_cells[i - 1][j];

        visited = make_unique<unique_ptr<bool[]>[]>(data.nr_rows);
        for (size_t index = 0; index < data.nr_rows; ++index)
        {
            visited[index] = make_unique<bool[]>(data.nr_columns);
            memset(visited[index].get(), 0, data.nr_columns);
        }
    }

    string key(int r, int c) {
        return to_string(r) + "," + to_string(c);
    }

    vector<std::pair<int, int>> get_spiral_pattern()
    {
        auto [start_row, start_col] = data.initial_cell;
        int total_cells = data.nr_rows * data.nr_columns;

        vector<std::pair<int, int>> result;
        unordered_set<string> visited_pos;

        // Directions: right, down, left, up
        vector<pair<int, int>> directions = {
                {0, 1}, {1, 0}, {0, -1}, {-1, 0}
        };

        int r = start_row, c = start_col;
        int dirIndex = 0; // starting direction
        int steps = 1;

        while (result.size() < total_cells)
        {
            for (int turn = 0; turn < 2; ++turn)
            {
                int dr = directions[dirIndex].first;
                int dc = directions[dirIndex].second;

                for (int i = 0; i < steps; ++i) {
                    if (r >= 0 && r < data.nr_rows && c >= 0 && c < data.nr_columns && visited_pos.find(key(r, c)) == visited_pos.end())
                    {
                        result.push_back({r, c});
                        visited_pos.insert(key(r, c));
                    }
                    r += dr;
                    c += dc;
                }

                dirIndex = (dirIndex + 1) % 4;
            }
            steps++; // increase spiral size every 2 directions
        }

        return result;
    }

    vector<vector<Point>> get_components()
    {
        vector<vector<Point>> components;
//        for (int i = 0; i < data.nr_rows; ++i)
//            for(int j = 0; j < data.nr_columns; ++j)
        for (auto [i, j]: get_spiral_pattern())
        {
            if (!visited[i][j] && data.building_plan[i][j] == '.')
            {
                const unsigned int router_pos_i = i + data.router_radius;
                const unsigned int router_pos_j = j + data.router_radius;
                if (router_pos_i < data.nr_rows && router_pos_j < data.nr_columns && can_place_router(router_pos_i, router_pos_j))
                {
                    auto current_component = apply_Lee(make_pair(router_pos_i, router_pos_j));
                    components.push_back(current_component);
                }
            }
        }

        sort(components.begin(), components.end(), [](const auto &lhs, const auto &rhs)
        {
            return lhs.size() > rhs.size();
        });

        return components;
    }
};