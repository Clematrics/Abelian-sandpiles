#pragma once

#include <cstdint>
// #include <mutex>
#include <queue>
#include <vector>

using grid = std::vector<std::vector<uint64_t>>;
using coords = std::pair<uint32_t, uint32_t>;

class sandpile {
public:
	sandpile(uint32_t width, uint32_t height);
	void reset();
	// void wait();
	// void changeSize(uint32_t width, uint32_t height);
	void addSand(uint32_t x, uint32_t y, uint64_t quantity);
	void setExpansionRule(grid r);
	uint64_t update(uint64_t number);
	void markGridToUpdate();
	uint64_t cellsToUpdate();
	std::vector<uint8_t> getGrid();
private:
	std::queue<coords> to_update;
	std::vector<std::vector<bool>> to_update_table;

	grid g;
	uint32_t g_width;
	uint32_t g_height;
	// std::mutex m;

	grid rule;
	uint64_t rule_cnt;
	uint32_t rule_width;
	uint32_t rule_height;
};