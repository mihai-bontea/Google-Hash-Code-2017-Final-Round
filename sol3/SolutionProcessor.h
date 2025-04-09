#pragma once
#define NMAX 99999999
#include <set>

#include "kdtree.hpp"

struct PointAccessor {
    typedef double result_type;
    double operator()(const Point& p, size_t dim) const {
        return dim == 0 ? p.first : p.second;
    }
};

class SolutionProcessor
{
private:
    Data& data;

public:

    static vector<Point> get_all_backbone_cells_between_points(const Point& router1, const Point& router2, const int nr_rows, const int nr_columns)
    {
        const int di[8] = { 0, 0, 1, -1, -1, 1, 1, -1 };
        const int dj[8] = { 1, -1, 0, 0, 1, -1, 1, -1 };

        vector<Point> result{router1};
        Point current_position = router1;

        while (current_position != router2)
        {
            Point closest_new_point;
            unsigned shortest_distance = NMAX;
            for (unsigned int direction = 0; direction < 8; ++direction)
            {
                const int new_i = (int)current_position.first + di[direction];
                const int new_j = (int)current_position.second + dj[direction];
                Point new_point = {new_i, new_j};
                const unsigned int new_distance = get_distance(new_point, router2);

                if (is_valid(new_point, nr_rows, nr_columns) && new_distance < shortest_distance)
                {
                    shortest_distance = new_distance;
                    closest_new_point = new_point;
                }
            }

            result.push_back(closest_new_point);
            current_position = closest_new_point;
        }
        return result;
    }

    explicit SolutionProcessor(Data& data): data{data}
    {
    }

    tuple<set<Point>, set<Point>, KDTree::KDTree<2, Point, PointAccessor>> process(const vector<Point>& raw_solution)
    {
        int remaining_budget = data.budget;

        KDTree::KDTree<2, Point, PointAccessor> tree;
        tree.insert(data.initial_cell);

        set<Point> backbone, routers;

        for (const auto& router : raw_solution)
        {
            auto [nearest_it, distance] = tree.find_nearest(router);
            auto backbone_cells_between_routers = get_all_backbone_cells_between_points(router, *nearest_it, data.nr_rows, data.nr_columns);
            const int cost_to_add = data.router_cost + backbone_cells_between_routers.size() * data.backbone_cost;

            if (cost_to_add <= remaining_budget)
            {
                routers.insert(router);
                tree.insert(router);
                for (const auto& cell : backbone_cells_between_routers)
                {
                    backbone.insert(cell);
                    tree.insert(cell);
                }
                remaining_budget -= cost_to_add;
            }
        }
        return {backbone, routers, tree};
    }
};