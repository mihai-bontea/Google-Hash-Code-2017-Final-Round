#pragma once
#include "Data.h"
#include "Definitions.h"
#define SUB_ROW_DIV 3
#define SUB_COL_DIV 4
#define NR_SUBMATR 12

class CoverageCalculator
{
private:
	const Data& data;
	unique_ptr<unique_ptr<unsigned int[]>[]> nr_walls;

	vector<Point> determine_covered_cells_for_position(Point router) const
	{
		const auto compute_rect_sum = [&](Matrix matrix)
		{
			// rectangle [(0, 0), (i2, j2)]
			unsigned int result = nr_walls[matrix.second.first][matrix.second.second];

			// subtracting [(0, 0), (i1 - 1, j2)], which is the rectangle above		(1)
			const int i1 = max(((int)matrix.first.first - 1), 0);
			result -= nr_walls[i1][matrix.second.second];

			// subtracting [(0, 0), (i2, j1 - 1)]									(2)
			const int j1 = max(((int)matrix.first.second - 1), 0);
			result -= nr_walls[matrix.second.first][j1];

			// Adding the intersection between (1) and (2) which has been subtracted twice
			//[(0, 0), (i1 - 1, j1 - 1)]
			return result + nr_walls[i1][j1];
		};

		const unsigned int i_min = (unsigned int)max((long long)router.first - (long long)data.router_radius, 0ll);
		const unsigned int j_min = (unsigned int)max((long long)router.second - (long long)data.router_radius, 0ll);
		const unsigned int i_max = min(router.first + data.router_radius, data.nr_rows - 1);
		const unsigned int j_max = min(router.second + data.router_radius, data.nr_columns - 1);

		vector<Point> covered_cells;
		for (unsigned int i = i_min; i <= i_max; ++i)
			for (unsigned int j = j_min; j <= j_max; ++j)
			{
				// Not a wall or area that doesn't require coverage => see if there's walls between router and current position
				if (data.building_plan[i][j] != '#' && data.building_plan[i][j] != '-')
				{
					const Point upper_left = make_pair(min(i, router.first), min(j, router.second));
					const Point lower_right = make_pair(max(i, router.first), max(j, router.second));
					//cout << upper_left.first << " " << upper_left.second << " " << lower_right.first << " " << lower_right.second << endl;

					if (!compute_rect_sum(make_pair(upper_left, lower_right)))
						covered_cells.push_back(make_pair(i, j));
				}
			}
		return covered_cells;
	}

	void determine_coverage_split(unique_ptr<unique_ptr<unsigned int[]>[]>& coverage, Matrix matrix) const
	{
		for (unsigned int i = matrix.first.first; i <= matrix.second.first; ++i)
			for (unsigned int j = matrix.first.second; j <= matrix.second.second; ++j)
				if (data.building_plan[i][j] != '#')
					coverage[i][j] = determine_covered_cells_for_position(make_pair(i, j)).size();
				else
					coverage[i][j] = 0;
					
	}

	array<Matrix, NR_SUBMATR> split_matrix() const
	{
		assert(data.nr_rows >= 12 && data.nr_columns >= 12);
		// Splitting the matrix into 12 sub-matrices
		array <Matrix, 12> sub_matrices;
		unsigned int submatrix_rows = data.nr_rows / SUB_ROW_DIV;
		unsigned int submatrix_cols = data.nr_columns / SUB_COL_DIV;

		unsigned index = 0;
		for (unsigned int upper_row = 0; upper_row < data.nr_rows - submatrix_rows; upper_row += (submatrix_rows - 1))
			for (unsigned int upper_col = 0; upper_col < data.nr_columns - submatrix_cols; upper_col += (submatrix_cols - 1))
			{
				unsigned int bottom_row = ((upper_row + 1 < data.nr_rows - submatrix_rows) ? (upper_row + submatrix_rows - 1) : (data.nr_rows - 1));
				unsigned int bottom_col = ((upper_col + 1 < data.nr_columns - submatrix_cols) ? (upper_col + submatrix_cols - 1) : (data.nr_columns - 1));

				assert(upper_row < data.nr_rows&& upper_col < data.nr_columns);
				assert(bottom_row < data.nr_rows&& bottom_col < data.nr_columns);

				sub_matrices[index++] = make_pair(make_pair(upper_row, upper_col), make_pair(bottom_row, bottom_col));
			}
		assert(index == 12);
		return sub_matrices;
	}

public:

	CoverageCalculator(const Data& data): data{data}
	{
		// Allocating memory
		nr_walls = make_unique<unique_ptr<unsigned int[]>[]>(data.nr_rows);
		for (unsigned int index = 0; index < data.nr_rows; ++index)
			nr_walls[index] = make_unique<unsigned int[]>(data.nr_columns);

		// Determining the nr of walls in any rectangle
		for (unsigned int i = 0; i < data.nr_rows; ++i)
			nr_walls[i][0] = (data.building_plan[i][0] == '#');

		// First, determining line sum
		for (unsigned int i = 0; i < data.nr_rows; ++i)
			for (unsigned int j = 1; j < data.nr_columns; ++j)
				nr_walls[i][j] = nr_walls[i][j - 1] + (data.building_plan[i][j] == '#');

		// Second, determining rectangle sum
		for (unsigned int j = 0; j < data.nr_columns; ++j)
			for (unsigned int i = 1; i < data.nr_rows; ++i)
				nr_walls[i][j] += nr_walls[i - 1][j];
	}

	unique_ptr<unique_ptr<unsigned int[]>[]> determine_coverage()
	{
		auto coverage = make_unique<unique_ptr<unsigned int[]>[]>(data.nr_rows);
		for (unsigned int index = 0; index < data.nr_rows; ++index)
			coverage[index] = make_unique<unsigned int[]>(data.nr_columns);

		// Split the matrix into 12 sub-matrices
		auto sub_matrices = split_matrix();

		array<thread, NR_SUBMATR> threads;
		for (unsigned int index = 0; index < NR_SUBMATR; ++index)
			threads[index] = thread(&CoverageCalculator::determine_coverage_split, this, ref(coverage), sub_matrices[index]);
		
		for (auto& th : threads)
			th.join();

		return coverage;
	}

	vector<Point> get_covered_cells(Point router)
	{
		return determine_covered_cells_for_position(router);
	}
};