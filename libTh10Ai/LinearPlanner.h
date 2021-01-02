#pragma once
#include "libTh10Ai/Bullet.h"
#include "libTh10Ai/Scene.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <math.h>
class LinearPlanner
{

private:
	int width;
	int height;
	int size = 2 + 4 + 4 + 4 + 25;
	std::vector<std::vector<double>> weights;

	int p_offset = 0;
	int grid_offset = 2;
	int min_d_offset = 27;
	int region_area_offset = 31;
	int region_dens_offset = 35;

public:
	LinearPlanner(int width, int height);
	~LinearPlanner();

	int initialize(std::string filename);
	int chooseAction(std::vector<th::Bullet>& bullets,  float player_x, float player_y);

};

