#pragma once
#define NMAX 99999999
#include <set>

#include "kdtree.hpp"

//struct Point {
//    double x, y;
//};

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

    static unsigned int get_distance(Point a, Point b)
    {
        const int x_dist = abs((int)a.first - (int)b.first);
        const int y_dist = abs((int)a.second - (int)b.second);

        return max(x_dist, y_dist);
    }

    [[nodiscard]] bool is_valid(Point point) const
    {
        return (point.first < data.nr_rows && point.second < data.nr_columns);
    }

    vector<Point> get_all_backbone_cells_between_points(const Point& router1, const Point& router2)
    {
        int di[8] = { 0, 0, 1, -1, -1, 1, 1, -1 };
        int dj[8] = { 1, -1, 0, 0, 1, -1, 1, -1 };

        vector<Point> result{router1};
        Point current_position = router1;

        while (current_position != router2)
        {
            Point closest_new_point;
            unsigned shortest_distance = NMAX;
            for (unsigned int direction = 0; direction < 8; ++direction)
            {
                unsigned int new_i = (int)current_position.first + di[direction];
                unsigned int new_j = (int)current_position.second + dj[direction];
                Point new_point = make_pair(new_i, new_j);
                unsigned int new_distance = get_distance(new_point, router2);

                if (is_valid(new_point) && new_distance < shortest_distance)
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

public:
    explicit SolutionProcessor(Data& data): data{data}
    {
    }

    pair<set<Point>, set<Point>> process(const vector<Point>& raw_solution)
    {
        int remaining_budget = data.budget;
//        std::cout << "Initial budget = " << remaining_budget << std::endl;

        KDTree::KDTree<2, Point, PointAccessor> tree;
        tree.insert(data.initial_cell);

        // Creating the backbone
        set<Point> backbone;
        set<Point> routers;

        for (const auto& router : raw_solution)
        {
            auto [nearest_it, distance] = tree.find_nearest(router);
            auto backbone_cells_between_routers = get_all_backbone_cells_between_points(router, *nearest_it);
            int cost_to_add = data.router_cost + backbone_cells_between_routers.size() * data.backbone_cost;

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
        backbone.erase(data.initial_cell);
//        std::cout << "Remaining budget = " << remaining_budget << "routers.size " << routers.size() << std::endl;
        return make_pair(backbone, routers);
    }
};


//#include "kdtree.hpp"
//#include <iostream>
//
//struct Point {
//    double x, y;
//};
//
//struct PointAccessor {
//    typedef double result_type;
//    double operator()(const Point& p, size_t dim) const {
//        return dim == 0 ? p.x : p.y;
//    }
//};
//
//int main() {
//    KDTree::KDTree<2, Point, PointAccessor> tree;
//    tree.insert({1.0, 2.0});
//    tree.insert({3.0, 4.0});
//    tree.insert({0.0, 0.0});
//
//    Point query = {2.5, 3.5};
//    auto nearest = tree.find_nearest(query);
//    std::cout << " " << nearest.first->x << " " << nearest.first->y;
////    std::cout << "Closest: (" << nearest.x << ", " << nearest.y << ")\n";
//}