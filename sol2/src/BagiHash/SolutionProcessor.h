#pragma once
#define NMAX 99999999
#include <set>

class SolutionProcessor
{
private:
	static unsigned int get_distance(Point a, Point b)
	{
		const int x_dist = abs((int)a.first - (int)b.first);
		const int y_dist = abs((int)a.second - (int)b.second);

		return max(x_dist, y_dist);
	}

	static bool is_valid(const Data &data, Point point)
	{
		return (point.first < data.nr_rows && point.second < data.nr_columns);
	}

	static vector<Point> get_all_backbone_cells_between_routers(const Data &data, Point router1, Point router2)
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

				if (is_valid(data, new_point) && new_distance < shortest_distance)
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
	static pair<set<Point>, set<Point>> process(const Data &data, const map<Point, Point>& raw_solution)
	{
		// Creating the backbone
		set<Point> backbone;
		set<Point> routers;
		
		for (const auto router_pair : raw_solution)
		{
			auto backbone_cells_between_routers = get_all_backbone_cells_between_routers(data, router_pair.first, router_pair.second);
			for (auto cell : backbone_cells_between_routers)
				backbone.insert(cell);
		}
		backbone.erase(data.initial_cell);

		// Creating the routers
		for (const auto router_pair : raw_solution)
			routers.insert(router_pair.first);

		return make_pair(backbone, routers);
	}
};