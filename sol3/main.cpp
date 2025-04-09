#include <omp.h>
#include <iostream>
#include <array>
#include <cassert>
#include <thread>

#include "Data.h"
#include "ComponentCalculator.h"
#include "SolutionProcessor.h"
#include "SafePriorityQueue.h"
#include "Definitions.h"

using namespace std;

class Solver
{
private:
    Data& data;
    int remaining_budget;
    unsigned int nr_cells_covered;
    unique_ptr<unique_ptr<unsigned int[]>[]> wall_map, nr_coverable_cells;
    unique_ptr<unique_ptr<bool[]>[]> visited;

    bool is_any_wall_between(const Point& point1, const Point& point2)
    {
        const Point upper_left = { std::min(point1.first, point2.first), std::min(point1.second, point2.second) };
        const Point bottom_right = { std::max(point1.first, point2.first), std::max(point1.second, point2.second) };

        return compute_rect_sum({upper_left, bottom_right}, wall_map);
    }

    void initialize_wall_map()
    {
        // Allocating memory
        wall_map = make_unique<unique_ptr<unsigned int[]>[]>(data.nr_rows);
        for (unsigned int index = 0; index < data.nr_rows; ++index)
            wall_map[index] = make_unique<unsigned int[]>(data.nr_columns);

        for (unsigned int i = 0; i < data.nr_rows; ++i)
            wall_map[i][0] = (data.building_plan[i][0] == '#');

        // First, determining line sum
        for (unsigned int i = 0; i < data.nr_rows; ++i)
            for (unsigned int j = 1; j < data.nr_columns; ++j)
                wall_map[i][j] = wall_map[i][j - 1] + (data.building_plan[i][j] == '#');

        // Second, determining rectangle sum
        for (unsigned int j = 0; j < data.nr_columns; ++j)
            for (unsigned int i = 1; i < data.nr_rows; ++i)
                wall_map[i][j] += wall_map[i - 1][j];
    }

    void initialize_coverable_cells()
    {
        #pragma omp parallel for
        for (int i = 0; i < data.nr_rows; ++i)
        {
            for (int j = 0; j < data.nr_columns; ++j)
            {
                // Could place a router here
                if (data.building_plan[i][j] == '.')
                {
                    int sum = 0;
                    for (int x = max(0, i - data.router_radius); x <= min(data.nr_rows - 1, i + data.router_radius); ++x)
                    {
                        for (int y = max(0, j - data.router_radius); y <= min(data.nr_columns - 1, j + data.router_radius); ++y)
                        {
                            if (data.building_plan[x][y] == '.' && !visited[x][y] && !is_any_wall_between({i, j}, {x, y}))
                                sum++;
                        }
                    }
                    nr_coverable_cells[i][j] = sum;
                }
                else
                    nr_coverable_cells[i][j] = 0;
            }
        }
    }

    void populate_pqueue_parallel(ThreadSafePriorityQueue& pq, const KDTree::KDTree<2, Point, PointAccessor>& tree)
    {
        #pragma omp parallel for
        for (int i = 0; i < data.nr_rows; ++i)
        {
            for (int j = 0; j < data.nr_columns; ++j)
            {
                if (data.building_plan[i][j] == '.')
                {

                    // Find the closest backbone/router to this position
                    const auto [nearest_it, _] = tree.find_nearest(make_pair(i, j));
                    const unsigned int distance = get_distance({i, j}, *nearest_it);

                    const int cost = (data.router_cost + data.backbone_cost * (int) distance);
                    const int score_gain = ((int) nr_coverable_cells[i][j] * 1000 - cost);

                    if (score_gain > 0 && cost <= remaining_budget)
                        pq.push({score_gain, {i, j}});
                }
            }
        }
    }

    void update_visited_and_coverage(const Point& point)
    {
        std::set<Point> newly_covered_points;

        // Update visited in an R radius around the point
        for (int i = point.first - data.router_radius; i <= point.first + data.router_radius; ++i)
        {
            for (int j = point.second - data.router_radius; j <= point.second + data.router_radius; ++j)
            {
                if (data.building_plan[i][j] == '.' && !is_any_wall_between(point, {i, j}) && !visited[i][j])
                {
                    newly_covered_points.insert({i, j});
                    visited[i][j] = true;
                }
            }
        }

        // Update nr_coverable_cells
        for (int i = max(0, point.first - 2 * data.router_radius); i <= min(data.nr_rows - 1, point.first + 2 * data.router_radius); ++i)
        {
            for (int j = max(0, point.second - 2 * data.router_radius); j <= min(data.nr_columns - 1, point.second + 2 * data.router_radius); ++j)
            {
                for (const auto& [x, y] : newly_covered_points)
                {
                    // Decrement the number of coverable cells for a theoretical router placed at (i, j)
                    // if (i, j) could cover (x, y)
                    if (data.building_plan[i][j] == '.' && !is_any_wall_between({i, j}, {x, y}) && nr_coverable_cells[i][j] > 0)
                        nr_coverable_cells[i][j]--;
                }
            }
        }
    }

    void add_new_routers(std::set<Point>& backbone, std::set<Point>& routers, KDTree::KDTree<2, Point, PointAccessor>& tree)
    {
        ThreadSafePriorityQueue pq;
        // This k-d tree will only contain routers, while the 'tree' variable contains backbone too
        KDTree::KDTree<2, Point, PointAccessor> all_routers;
        for (const auto& router : routers)
            all_routers.insert(router);

        while (remaining_budget >= 0)
        {
            populate_pqueue_parallel(pq, tree);
            KDTree::KDTree<2, Point, PointAccessor> new_routers_tree;

            bool router_was_added = false;
            while (!pq.empty())
            {
                std::pair<int, Point> res;
                pq.pop(res);
                auto [est_score_per_point, new_router_point] = res;

                const auto [closest_router_it, _] = new_routers_tree.find_nearest(new_router_point);
                const auto closest_router_distance = get_distance(new_router_point, *closest_router_it);

                const auto [closest_old_router_it, __] = all_routers.find_nearest(new_router_point);
                const auto closest_old_router_distance = get_distance(new_router_point, *closest_old_router_it);

                // No overlap with newly added routers
                if ((new_routers_tree.empty() || closest_router_distance > 2 * data.router_radius) &&
                        closest_old_router_distance >= 2)
                {
                    // Find the closest backbone/router to connect it to
                    const auto [closest_backbone_it, _] = tree.find_nearest(new_router_point);

                    auto backbone_cells_between_routers =
                            SolutionProcessor::get_all_backbone_cells_between_points(new_router_point,
                                                                                     *closest_backbone_it,
                                                                                     data.nr_rows,
                                                                                     data.nr_columns);

                    const int approx_cost_to_add = data.router_cost + backbone_cells_between_routers.size() * data.backbone_cost;

                    // Add the new router if we can afford it
                    if (approx_cost_to_add <= remaining_budget)
                    {
                        routers.insert(new_router_point);
                        tree.insert(new_router_point);
                        new_routers_tree.insert(new_router_point);
                        all_routers.insert(new_router_point);

                        int actual_backbone_needed = 0;
                        for (const auto &cell: backbone_cells_between_routers)
                        {
                            auto [_, insertion_happened] = backbone.insert(cell);
                            if (insertion_happened)
                                actual_backbone_needed++;

                            tree.insert(cell);
                        }
                        // Some of the backbone cells might already be inserted
                        const int exact_cost_to_add = data.router_cost + actual_backbone_needed * data.backbone_cost;
                        remaining_budget -= exact_cost_to_add;
                        nr_cells_covered += nr_coverable_cells[new_router_point.first][new_router_point.second];

                        // Update visited[i][j] in radius R, and nr_coverable_cells in radius 2 * R
                        // with respect to the newly added router
                        update_visited_and_coverage(new_router_point);

                        // Something was added this iteration, so it's worth to re-populate the pq
                        router_was_added = true;
                    }
                }
            }
            if (!router_was_added)
                break;
        }
    }

public:

    explicit Solver(Data&  data)
    : data(data)
    , remaining_budget(data.budget)
    , nr_cells_covered(0)
    {
        initialize_wall_map();

        // Allocate memory and initialize the visited map to false
        visited = make_unique<unique_ptr<bool[]>[]>(data.nr_rows);
        for (size_t index = 0; index < data.nr_rows; ++index)
        {
            visited[index] = make_unique<bool[]>(data.nr_columns);
            memset(visited[index].get(), 0, data.nr_columns);
        }

        // Allocate memory for the coverable cells map
        nr_coverable_cells = make_unique<unique_ptr<unsigned int[]>[]>(data.nr_rows);
        for (unsigned int index = 0; index < data.nr_rows; ++index)
            nr_coverable_cells[index] = make_unique<unsigned int[]>(data.nr_columns);
    }

    tuple<set<Point>, set<Point>> solve()
    {
        // Step 1: obtain the 'perfect' router locations (no walls in range, no overlap whatsoever)
        // (also updates the visited map)
        auto comp_calc = std::make_unique<ComponentCalculator>(data, visited);
        const auto perfect_routers = comp_calc->get_perfect_routers();
        comp_calc.reset();

        // Step 2: process the routers so far, obtain the backbone coords, and a k-d tree containing the routers
        auto sol_proc = std::make_unique<SolutionProcessor>(data);
        auto [backbone, routers, tree] = sol_proc->process(perfect_routers);
        assert(perfect_routers.size() == routers.size());
        sol_proc.reset();

        // Update the cells covered, and the remaining budget after adding the perfect routers
        nr_cells_covered = routers.size() * (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1);
        remaining_budget = data.budget - (routers.size() * data.router_cost + backbone.size() * data.backbone_cost);

        // Initialize coverable cells map based on the perfect routers found so far
        initialize_coverable_cells();

        // Step 3: add new routers
        add_new_routers(backbone, routers, tree);

        auto to_set = [](const auto& container){
            return std::set(container.begin(), container.end());
        };

        // Remove the original backbone start, since it is redundant to add it to output
        backbone.erase(data.initial_cell);
        return {to_set(backbone), to_set(routers)};
    }
};

int main()
{
    const string in_prefix = "../../input_files/";
    const string out_prefix = "../../output_files/sol3/";

    const array<string, 4> input_files = { "charleston_road.in", "lets_go_higher.in", "opera.in", "rue_de_londres.in" };
    for (const string& input_file : input_files)
    {
        cout << "Now working on " << input_file << std::endl;
        Data data(in_prefix + input_file);
        Solver solver(data);
        auto [backbone, routers] = solver.solve();

        data.write_to_file(out_prefix + input_file.substr(0, (input_file.find('.'))), backbone, routers);
    }
    return 0;
}