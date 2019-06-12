#include "sandpiles.hpp"

#include <algorithm>
#include <numeric>

sandpile::sandpile(uint32_t width, uint32_t height) {
	to_update = { };
	to_update_table = std::vector<std::vector<bool>>(width, std::vector<bool>(height, false));

	g_width = width;
	g_height = height;
	g = std::vector<std::vector<uint64_t>>(g_width, std::vector<uint64_t>(g_height, 0));

	rule_width = 3;
	rule_height = 3;
	rule = { { 0, 1, 0 }, { 1, 0, 1 }, { 0, 1, 0 } };
	rule_cnt = 4;
}

void sandpile::reset() {
	g = std::vector<std::vector<uint64_t>>(g_width, std::vector<uint64_t>(g_height, 0));
}

void sandpile::setExpansionRule(grid r) {
	rule = r;
	rule_width = r.size();
	rule_height = r[0].size();
	std::vector<uint64_t> partial_sum(rule_width, 0);
	std::transform(r.begin(), r.end(), partial_sum.begin(), [](auto row){ return std::accumulate(row.begin(), row.end(), 0); });
	rule_cnt = std::accumulate(partial_sum.begin(), partial_sum.end(), 0);
}

void sandpile::addSand(uint32_t x, uint32_t y , uint64_t quantity) {
	g[x][y] += quantity;
	to_update.push( {x, y} );
	to_update_table[x][y] = true;
}

uint64_t sandpile::cellsToUpdate() {
	return to_update.size();
}

void sandpile::markGridToUpdate() {
	for (uint32_t x = 0; x < g_width; ++x) {
		for (uint32_t y = 0; y < g_height; ++y) {
			to_update.push( {x, y} );
			to_update_table[x][y] = true;
		}
	}
}

uint64_t sandpile::update(uint64_t number) {
	for (uint64_t i = 0; i < number && !to_update.empty(); ++i) {
		auto [x, y] = to_update.front();
		to_update.pop();
		to_update_table[x][y] = false;

		if (g[x][y] < rule_cnt)
			continue;
		uint64_t quantity = g[x][y];
		uint64_t left = quantity % rule_cnt;
		uint64_t factor = quantity / rule_cnt;

		g[x][y] = left;
		for (int32_t rule_x = 0, offset_x = - ((int32_t)rule_width) / 2; rule_x < rule_width ; ++rule_x, ++offset_x) {
			for (int32_t rule_y = 0, offset_y = - ((int32_t)rule_height) / 2 ; rule_y < rule_height ; ++rule_y, ++offset_y) {
				auto x_ = x + offset_x;
				auto y_ = y + offset_y;
				if (x_ < 0 || y_ < 0 || x_ >= g_width || y_ >= g_height)
					continue;
				g[x_][y_] += factor * rule[rule_x][rule_y];
				if (!to_update_table[x_][y_]) {
					to_update_table[x_][y_] = true;
					to_update.push( {x_ , y_ } );
				}
			}
		}
	}

	return cellsToUpdate();
}

std::vector<uint8_t> sandpile::getGrid() {
	std::vector<uint8_t> data(g_width * g_height * 4);
	for (uint32_t x = 0; x < g_width; ++x) {
		for (uint32_t y = 0; y < g_height; ++y) {
			data[4 * (x * g_height + y) + 0] = 255 * (g[x][y] % rule_cnt) / rule_cnt;
			data[4 * (x * g_height + y) + 1] = 255 * (g[x][y] % rule_cnt) / rule_cnt;
			data[4 * (x * g_height + y) + 2] = 255 * (g[x][y] % rule_cnt) / rule_cnt;
			data[4 * (x * g_height + y) + 3] = 255;
		}
	}
	return data;
}