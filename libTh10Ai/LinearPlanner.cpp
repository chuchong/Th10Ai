#include "LinearPlanner.h"



LinearPlanner::LinearPlanner(int width, int height):width(width), height(height)
{
}

LinearPlanner::~LinearPlanner()
{
}

int LinearPlanner::initialize(std::string filename)
{
	std::cout << "read start" << std::endl;
	this->weights.resize(4);
	std::ifstream  f(filename.c_str(), std::ios::in);
	if (!f.is_open()) {
		std::cout << "cannot open file " << filename << std::endl;
		GetLastError();
		return 1;
	}

	while (!f.eof()) {
		for (int i = 0; i < 4; i++) {
			double var;
			f >> var;
			this->weights[i].push_back(var);
		}
	}
	std::cout << "read down"  << std::endl;
	return 0;
}

int LinearPlanner::chooseAction(std::vector<th::Bullet>& bullets, float player_x, float player_y)
{

	player_x += (this->width / 2.0f);
	
	std::vector<double> feature_map;
	feature_map.resize(this->size);

	feature_map[p_offset + 0] = player_x / this->width;
	feature_map[p_offset + 1] = player_y / this->height;

	double s = this->width * this->height;
	feature_map[region_area_offset + 0] = (this->width - player_x) * (this->height - player_y) / s;
	feature_map[region_area_offset + 1] = (this->width) * (this->height - player_y) / s;
	feature_map[region_area_offset + 2] = (this->width) * (this->height) / s;
	feature_map[region_area_offset + 3] = (this->width - player_x) * (this->height) / s;

	feature_map[min_d_offset + 0] = feature_map[min_d_offset + 1] = feature_map[min_d_offset + 2] = feature_map[min_d_offset + 3] = 1;

	for (const th::Bullet& bullet : bullets) {
		if (!th::Scene::IsInScene(bullet.getPosition())) {
			continue;
		}
		float x = bullet.x + (th::Scene::SIZE.width / 2.0f);
		float y = bullet.y;
		
		int i = std::min(int(x) / (int)(this->width / 5.0), 4);
		int j = std::min(int(y) / (int)(this->height / 5.0), 4);
		feature_map[grid_offset + i * 5 + j] += 1;

		int bin = 0;
		if (x >= player_x && y >= player_y) {
			bin = 0;
		}
		else if (x < player_x && y >= player_y) {
			bin = 1;
		}
		else if (x < player_x && y < player_y) {
			bin = 2;
		}
		else {
			bin = 3;
		}
		feature_map[bin + region_dens_offset] += 1;

		float dist = (abs(x - player_x) + abs(y - player_y)) / (this->width + this->height);
		if (dist < feature_map[bin + min_d_offset]) {
			feature_map[bin + min_d_offset] = dist;
		}
	}

	for (int i = 0; i < 25; i++) {
		feature_map[i + grid_offset] = log2f(feature_map[i + grid_offset] + 1);
	}
	for (int i = 0; i < 4; i++) {
		feature_map[i + region_dens_offset] /= (feature_map[i + region_area_offset] * s);
	}

	//std::cout << "feature_map" << std::endl;
	//for (int i = 0; i < size; i++) std::cout << "f:" << feature_map[i] << std::endl;

	int best_action = 3;
	double best_reward = -1000000;
	for (int action = 0; action < 4; action++) {
		double reward = 0;
		for (int i = 0; i < size; i++) {
			reward += this->weights[action][i] * feature_map[i];
		}

		if (reward > best_reward) {
			best_action = action;
			best_reward = reward;
		}
	}

	return best_action;
}
