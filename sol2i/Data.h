#pragma once
#include <set>
#include <iostream>
#include <fstream>

#include "Definitions.h"

using namespace std;

class Data
{
private:

public:

    int nr_rows, nr_columns, router_radius, backbone_cost, router_cost, budget;
    Point initial_cell;
    unique_ptr<unique_ptr<char[]>[]> building_plan;


    Data(const string& filename)
    {
        ifstream fin(filename);

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

    void write_to_file(const string& filename, const set<Point>& backbone, const set<Point>& routers) const
    {
        ofstream fout(filename);
        fout << backbone.size() << '\n';
        for (const auto& cell : backbone)
            fout << cell.first << " " << cell.second << '\n';

        fout << routers.size() << '\n';
        for (const auto& router : routers)
            fout << router.first << " " << router.second << '\n';
    }
};