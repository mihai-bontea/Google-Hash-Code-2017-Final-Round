#pragma once
#include <queue>
#include <cstring>
#include <algorithm>
#include <unordered_set>

#include "Data.h"
#include "Definitions.h"

class ComponentCalculator
{
private:
    const Data& data;
    unique_ptr<unique_ptr<unsigned int[]>[]> nr_coverable_cells;
    unique_ptr<unique_ptr<bool[]>[]>& visited;

    bool can_place_router(int i, int j)
    {
        const int upper_left_i = i - data.router_radius;
        const int upper_left_j = j - data.router_radius;

        const int bottom_right_i = i + data.router_radius;
        const int bottom_right_j = j + data.router_radius;

        // bound-checking
        if (!is_valid({upper_left_i, upper_left_j}, data.nr_rows, data.nr_columns) ||
            !is_valid({bottom_right_i, bottom_right_j}, data.nr_rows, data.nr_columns))
                return false;

        // checking if all cells are unvisited
        for (int i1 = upper_left_i; i1 <= bottom_right_i; ++i1)
            for (int j1 = upper_left_j; j1 <= bottom_right_j; ++j1)
                if (visited[i1][j1])
                    return false;

        const Point upper_left = make_pair(upper_left_i, upper_left_j);
        const Point bottom_right = make_pair(bottom_right_i, bottom_right_j);
        const auto nr_coverable_cells_in_matrix = compute_rect_sum({upper_left, bottom_right}, nr_coverable_cells);
        return nr_coverable_cells_in_matrix == (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1);
    };

    vector<Point> apply_Lee(const Point& initial_router)
    {
        auto are_coords_valid = [&](const int i, const int j)
        {
            return (i >= 0 && j >= 0 && i < data.nr_rows&& j < data.nr_columns);
        };

        auto add_as_visited = [&](const Point& router_position)
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
                    const Point new_router = {new_i, new_j};
                    add_as_visited(new_router);
                    routers.push(new_router);
                    result.push_back(new_router);
                }
            }
        }

        return result;
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
                        result.emplace_back(r, c);
                        visited_pos.insert(key(r, c));
                    }
                    r += dr;
                    c += dc;
                }

                dirIndex = (dirIndex + 1) % 4;
            }
            steps++;
        }

        return result;
    }

public:

    ComponentCalculator(const Data& data, unique_ptr<unique_ptr<bool[]>[]>& visited)
    : data(data)
    , visited(visited)
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
    }

    vector<Point> get_perfect_routers()
    {
        vector<Point> perfect_routers;
        for (const auto& [i, j]: get_spiral_pattern())
        {
            if (!visited[i][j] && data.building_plan[i][j] == '.')
            {
                const unsigned int router_pos_i = i + data.router_radius;
                const unsigned int router_pos_j = j + data.router_radius;
                if (router_pos_i < data.nr_rows && router_pos_j < data.nr_columns && can_place_router(router_pos_i, router_pos_j))
                {
                    const auto routers_in_component = apply_Lee(make_pair(router_pos_i, router_pos_j));
                    perfect_routers.insert(perfect_routers.end(), routers_in_component.begin(), routers_in_component.end());
                }
            }
        }

        return perfect_routers;
    }
};