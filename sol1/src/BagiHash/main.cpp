#include <iostream>
#include <array>
#include <cassert>
#include <thread>
#include <cstring>
#include <cmath>
#include <deque>
#include <map>
#include "Data.h"
#include "SegTree2D.h"
#include "Definitions.h"
#include "CoverageCalculator.h"
#include "SolutionProcessor.h"
#include <thread>
#include <array>
#include <mutex>
#define NR_THREADS 18

using namespace std;
unsigned long long final_score = 0;

class Solver
{
private:
	const Data& data;
	SegTree2D& st;
	CoverageCalculator& coverage_calculator;
	unique_ptr<unique_ptr<bool[]>[]> is_covered;
	unsigned int nr_cells_covered = 0;

	Matrix get_matrix(Point middle, unsigned int radius) const
	{
		unsigned int upper_left_line = ((((int)middle.first - (int)radius + 1 ) >= 0)? (middle.first - radius + 1) : 0);
		unsigned int upper_left_col = ((((int)middle.second - (int)radius + 1) >= 0) ? (middle.second - radius + 1) : 0);

		unsigned int lower_right_line = ((((int)middle.first + (int)radius - 1) < data.nr_rows) ? (middle.first + radius - 1) : (data.nr_rows - 1));
		unsigned int lower_righ_col = ((((int)middle.second + (int)radius - 1) < data.nr_columns) ? (middle.second + radius - 1) : (data.nr_columns - 1));

		return make_pair(make_pair(upper_left_line, upper_left_col), make_pair(lower_right_line, lower_righ_col));
	}

	unsigned int get_actual_coverage(query_result qr) const
	{
		auto cells_covered_by_router = coverage_calculator.get_covered_cells(qr.second);
		unsigned int overlap = 0;
		for (auto cell : cells_covered_by_router)
			if (is_covered[cell.first][cell.second])
				++overlap;
		return cells_covered_by_router.size() - overlap;
	}

	unsigned int get_distance(Point a, Point b) const
	{
		const int x_dist = abs((int)a.first - (int)b.first);
		const int y_dist = abs((int)a.second - (int)b.second);

		return max(x_dist, y_dist);
	}

public:
	Solver(const Data& data, SegTree2D& st, CoverageCalculator &coverage_calculator): data{data}, st{st}, coverage_calculator{coverage_calculator}
	{
		is_covered = make_unique<unique_ptr<bool[]>[]>(data.nr_rows);
		for (size_t index = 0; index < data.nr_rows; ++index)
		{
			is_covered[index] = make_unique<bool[]>(data.nr_columns);
			memset(is_covered[index].get(), 0, data.nr_columns);
		}
	}

	map<Point, Point> solve()
	{
		unsigned int remaining_budget = data.budget;
		map<Point, Point> routers_in_solution;
		deque<Point> router_queue;
		router_queue.push_front(data.initial_cell);
		while(true)
		{
			// Find the best position for a new router in the general area of the current router
			optional<pair<query_result, double>> best_result;
			Point best_result_maps_to;
			assert(!best_result.has_value());
			unsigned int best_result_cost = 0;
			
			int count = 0;
			// Go over the routers already placed(and the initial cell of the backbone)
			for (auto router : router_queue)
			{
				mutex modify_best_result_mtx;
				auto get_best_result_for_given_radius = [&](unsigned int radius)
				{
					Matrix matrix = get_matrix(router, radius);
					query_result matrix_max = st.get_max(matrix);

					// if this router wasn't added before
					if (routers_in_solution.find(matrix_max.second) == routers_in_solution.end())
					{
						unsigned int actual_coverage = get_actual_coverage(matrix_max);
						const unsigned int distance = get_distance(router, matrix_max.second);
						const unsigned int placement_cost = distance * data.backbone_cost + data.router_cost;

						double score = actual_coverage * 0.60 + ((200 - distance) / 2 * 0.40); // 69

						modify_best_result_mtx.lock();
						if ((placement_cost <= remaining_budget) && (!best_result.has_value() || score > best_result.value().second))
						{
							best_result = make_pair(matrix_max, score);
							best_result_cost = placement_cost;
							best_result_maps_to = router;
						}
						modify_best_result_mtx.unlock();
					}
				};

				unsigned int radius = 10;
				array<thread, NR_THREADS> compute_best_result_th;
				for (size_t index = 0; index < NR_THREADS; ++index)
				{
					compute_best_result_th[index] = thread(get_best_result_for_given_radius, radius);
					radius += 3;
				}

				for (auto& th : compute_best_result_th)
					th.join();
					
				count++;
				if (count >= 10 && best_result.has_value())
					break;
			}
			if (best_result.has_value())
			{
				routers_in_solution.insert(make_pair(best_result.value().first.second, best_result_maps_to));
				router_queue.push_front(best_result.value().first.second);
				remaining_budget -= best_result_cost;
				st.update(best_result.value().first.second);

				auto cells_covered_by_router = coverage_calculator.get_covered_cells(best_result.value().first.second);
				for (auto cell : cells_covered_by_router)
					if (!is_covered[cell.first][cell.second])
					{
						is_covered[cell.first][cell.second] = true;
						++nr_cells_covered;
					}
			}
			else
				break;
		}
		unsigned long long score = 1000 * nr_cells_covered + remaining_budget;
		final_score += score;
		cout << "Cells covered: " << nr_cells_covered << ", Score: " << score << "\n\n";
		return routers_in_solution;
	}
};

int main()
{
	const string in_prefix = "../../../input_files/";
	const string out_prefix = "../../../output_files/sol1/";

	const array<string, 4> input_files = { "charleston_road.in", "lets_go_higher.in", "opera.in", "rue_de_londres.in" };

	for (string input_file : input_files)
	{
		cout << "Now working on " << input_file;
		Data data(in_prefix + input_file);
		cout << ". Input processed.\n";
		cout << data.router_radius << '\n';
		
		CoverageCalculator coverage_calculator(data);
		//  coverage[i][j] == x means that if you'd put a router at coords (i, j) it would cover x blocks
		auto coverage = coverage_calculator.determine_coverage();

		SegTree2D st(coverage, data.nr_rows, data.nr_columns);
		Solver solver(data, st, coverage_calculator);
		coverage.reset();

		auto raw_solution = solver.solve();
		auto processed_solution = SolutionProcessor::process(data, raw_solution);
		data.write_to_file(out_prefix + input_file.substr(0, (input_file.find('.'))), processed_solution);
	}
	cout << "Final score = " << final_score << '\n';
	return 0;
}