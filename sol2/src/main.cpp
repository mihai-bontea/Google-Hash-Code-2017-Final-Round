#include <iostream>
#include <array>
#include <cassert>
#include <thread>
#include <cstring>
#include <cmath>
#include <deque>
#include <map>
#include "Data.h"
#include "Definitions.h"
#include "ComponentCalculator.h"
#include "SolutionProcessor.h"
#include <thread>
#include <array>
#include <optional>
#include <algorithm>
#include <list>
#define NR_THREADS 18

using namespace std;
unsigned long long final_score = 0;

class Solver
{
private:
	const Data& data;
	list<map<Point, Point>> components;

	unsigned int get_distance(Point a, Point b) const
	{
		const int x_dist = abs((int)a.first - (int)b.first);
		const int y_dist = abs((int)a.second - (int)b.second);

		return max(x_dist, y_dist);
	}

	unsigned int compute_component_cost(map<Point, Point> component) const
	{
		auto starting_router_it = find_if(component.begin(), component.end(), [&](const auto router_to_parent)
			{
				return router_to_parent.second == data.initial_cell;
			});

		assert(starting_router_it != component.end());

		// The cost of connecting the initial cell of the component
		unsigned int cost = get_distance(data.initial_cell, starting_router_it->first) * data.backbone_cost;

		// The cost of all the routers in the component
		cost += component.size() * data.router_cost;
		
		// The cost of connecting the routers to the backbone
		cost += (2 * data.router_radius + 1) * (component.size() - 1);

		return cost;
	}

public:
	Solver(const Data& data, vector<map<Point, Point>> components): data{data}, components{components.begin(), components.end()}
	{
	}

	map<Point, Point> solve()
	{
		unsigned int remaining_budget = data.budget;
		map<Point, Point> routers_in_solution;

		for (auto component : components)
		{
			auto cost = compute_component_cost(component);
			double score = (double)(component.size() * (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1)) / cost;
			if (cost <= remaining_budget)
			{
				routers_in_solution.insert(component.begin(), component.end());
				remaining_budget -= cost;
			}
		}
		
		const unsigned int nr_cells_covered = routers_in_solution.size() * (data.router_radius * 2 + 1) * (data.router_radius * 2 + 1);
		unsigned long long score = 1000 * nr_cells_covered + remaining_budget;
		final_score += score;
		cout << "Remaining budget = " << remaining_budget << "\n";
		cout << "Cells covered: " << nr_cells_covered << ", Score: " << score << "\n\n";
		return routers_in_solution;
	}
};

int main()
{
	const string in_prefix = "../../../input_files/";
	const string out_prefix = "../../../output_files/sol2/";

	const array<string, 4> input_files = { "charleston_road.in", "lets_go_higher.in", "opera.in", "rue_de_londres.in" };

	for (string input_file : input_files)
	{
		cout << "Now working on " << input_file;
		Data data(in_prefix + input_file);
		cout << ". Input processed.\n";
		
		ComponentCalculator component_calculator(data);
		
		auto components = component_calculator.get_components();

		Solver solver(data, components);

		auto raw_solution = solver.solve();
		auto processed_solution = SolutionProcessor::process(data, raw_solution);
		data.write_to_file(out_prefix + input_file.substr(0, (input_file.find('.'))), processed_solution);
	}
	cout << "Final score = " << final_score << '\n';
	return 0;
}