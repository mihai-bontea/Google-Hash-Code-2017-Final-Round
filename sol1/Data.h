#pragma once
#include <iostream>
#include <fstream>
#include <set>
#include "InParser.h"
#include "Definitions.h"

using namespace std;

class Data
{
private:

public:

	unsigned int nr_rows, nr_columns, router_radius, backbone_cost, router_cost, budget;
	pair<unsigned int, unsigned int> initial_cell;
	unique_ptr<unique_ptr<char[]>[]> building_plan;


	Data(const string filename)
	{
		//ifstream fin(filename);
		InParser fin(filename);

		fin >> nr_rows >> nr_columns >> router_radius >> backbone_cost >> router_cost >> budget;
		fin >> initial_cell.first >> initial_cell.second;

		// Allocating memory for the building plan
		building_plan = make_unique<unique_ptr<char[]>[]>(nr_rows);
		for (unsigned int index = 0; index < nr_rows; ++index)
			building_plan[index] = make_unique<char[]>(nr_columns);

		// Reading the building plan
		for (unsigned int i = 0; i < nr_rows; ++i)
			for (unsigned int j = 0; j < nr_columns; ++j)
				fin >> building_plan[i][j];
	}

	void write_to_file(const string filename, pair<set<Point>, set<Point>> solution)
	{
		ofstream fout(filename);
		auto backbone = solution.first;
		fout << backbone.size() << '\n';
		for (auto cell : backbone)
			fout << cell.first << " " << cell.second << '\n';

		auto routers = solution.second;
		fout << routers.size() << '\n';
		for (auto router : routers)
			fout << router.first << " " << router.second << '\n';
		fout.close();
	}
};