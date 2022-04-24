#pragma once
#include <iostream>
#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>
#include "Definitions.h"

class SegTree2D
{
private:
	std::unique_ptr < std::unique_ptr<query_result[]>[]> seg_tree;
	size_t n, m;

	inline void build_tree(size_t tree_index, const std::unique_ptr<std::unique_ptr<unsigned int[]>[]>& mat, size_t node, size_t left, size_t right)
	{
		if (left == right)
			seg_tree[tree_index][node] = std::make_pair(mat[tree_index][left - 1], std::make_pair(tree_index, left - 1));
		else
		{
			size_t mid = (left + right) / 2;
			build_tree(tree_index, mat, 2 * node, left, mid);
			build_tree(tree_index, mat, 2 * node + 1, mid + 1, right);
			
			const query_result left_res = seg_tree[tree_index][2 * node];
			const query_result right_res = seg_tree[tree_index][2 * node + 1];

			seg_tree[tree_index][node] = ((left_res.first > right_res.first) ? left_res : right_res);
		}
	}

	inline query_result get_max_subtree(size_t tree_index, size_t node, size_t left, size_t right, size_t x, size_t y) const
	{
		if (x <= left && right <= y)
			return seg_tree[tree_index][node];
		
		size_t mid = (left + right) / 2;

		std::optional<query_result> left_res, right_res;
		if (x <= mid)
			left_res = get_max_subtree(tree_index, 2 * node, left, mid, x, y);
		if (y > mid)
			right_res = get_max_subtree(tree_index, 2 * node + 1, mid + 1, right, x, y);

		//cout << left << " " << right << "| " << x << " " << y << endl;

		if (!left_res.has_value())
		{
			assert(right_res.has_value());
			return right_res.value();
		}
		else if (!right_res.has_value())
		{
			assert(left_res.has_value());
			return left_res.value();
		}
		else
			return ((left_res.value().first > right_res.value().first) ? left_res.value() : right_res.value());
	}

	inline void update_subtree(size_t tree_index, size_t node, size_t left, size_t right, size_t x)
	{
		if (left == right)
			seg_tree[tree_index][node].first = 0;
		else
		{
			size_t mid = (left + right) / 2;
			if (x <= mid)
				update_subtree(tree_index, 2 * node, left, mid, x);
			else
				update_subtree(tree_index, 2 * node + 1, mid + 1, right, x);

			const query_result left_res = seg_tree[tree_index][2 * node];
			const query_result right_res = seg_tree[tree_index][2 * node + 1];
			seg_tree[tree_index][node] = ((left_res.first > right_res.first) ? left_res : right_res);
		}
	}

public:
	SegTree2D(const std::unique_ptr<std::unique_ptr<unsigned int[]>[]>& mat, size_t _n, size_t _m): n{_n}, m{_m}
	{
		// Allocate memory for a segment tree for each line of the matrix
		seg_tree = std::make_unique<std::unique_ptr<query_result[]>[]>(n);
		for (size_t index = 0; index < n; ++index)
			seg_tree[index] = std::make_unique<query_result[]>(4 * (m + 1));

		// Build the segment trees
		for (size_t index = 0; index < n; ++index)
			build_tree(index, mat, 1, 1, m);
	}

	query_result get_max(Matrix mat_coords) const
	{
		//cout << mat_coords.first.second << " " << mat_coords.second.second << endl;
		assert(mat_coords.first.first < mat_coords.second.first);
		assert(mat_coords.first.second < mat_coords.second.second);
		assert(mat_coords.second.first <= n);
		assert(mat_coords.second.second <= m);

		std::vector<query_result> max_line_res;
		for (size_t index = mat_coords.first.first; index <= mat_coords.second.first; ++index)
			max_line_res.push_back(get_max_subtree(index, 1, 1, m, mat_coords.first.second + 1, mat_coords.second.second + 1));

		const query_result max_res = *std::max_element(max_line_res.begin(), max_line_res.end(), [](const query_result lhs, const query_result rhs) {
			return lhs.first < rhs.first;
			});

		return max_res;
	}

	void update(Point point)
	{
		update_subtree(point.first, 1, 1, m, point.second + 1);
	}
};