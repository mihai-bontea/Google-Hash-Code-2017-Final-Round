#include <iostream>
#include <array>
#include <cassert>
#include <thread>

#include "Data.h"
#include "ComponentCalculator.h"
#include "SolutionProcessor.h"

using namespace std;
unsigned long long final_score = 0;

int main()
{
    const string in_prefix = "../../input_files/";
    const string out_prefix = "../../output_files/sol2i/";

    const array<string, 4> input_files = { "charleston_road.in", "lets_go_higher.in", "opera.in", "rue_de_londres.in" };
    for (const string& input_file : input_files)
    {
        cout << "Now working on " << input_file;
        Data data(in_prefix + input_file);

        ComponentCalculator component_calculator(data);
        const auto components = component_calculator.get_components();

        vector<Point> routers_to_try;
        for (auto& component : components)
            for (auto& router : component)
                routers_to_try.push_back(router);

        SolutionProcessor sp(data);
        const auto processed_solution = sp.process(routers_to_try);
        const auto& [backbone, routers] = processed_solution;

        const int nr_cells_covered = routers.size() * (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1);
        const int remaining_budget = data.budget - (routers.size() * data.router_cost + backbone.size() * data.backbone_cost);
        assert(remaining_budget >= 0);
        const int score = 1000 * nr_cells_covered + remaining_budget;
        cout << "Cells covered: " << nr_cells_covered << ", Score: " << score << "\n\n";
        final_score += score;

        data.write_to_file(out_prefix + input_file.substr(0, (input_file.find('.'))), backbone, routers);
    }
    cout << "Final score = " << final_score << '\n';
    return 0;
}