
#include <chrono>
#include <iostream>
#include <thread>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

#include "application_log.hpp"
#include "sandpiles.hpp"

uint32_t width = 361;
uint32_t height = 361;

bool worker_running = true;
bool continue_sim = true;

sandpile sp(width, height);

void sandpile_loop() {
update:
	worker_running = true;
	uint64_t nb_to_update = 1;
	while (nb_to_update > 0 && continue_sim) {
		auto start = std::chrono::high_resolution_clock::now();
		nb_to_update = sp.update(1000000);
		auto end = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		logDebug(std::to_string(diff.count()) + "ms");
	}

	worker_running = false;
	while (continue_sim) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (nb_to_update = sp.cellsToUpdate())
			goto update;
	}
}

void showSandpileControl() {

	if (ImGui::CollapsingHeader("Sand")) {
		static int x = width / 2, y = height / 2, quantity = 1;

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::InputInt("x", &x); x = x < 0 ? 0 : x >= width ? width : x;
		ImGui::InputInt("y", &y); y = y < 0 ? 0 : y >= height ? height : y;
		ImGui::InputInt("Quantity", &quantity);

		if (ImGui::Button("Add sand")) {
			sp.addSand(x, y, quantity);
		}
		if (ImGui::Button("Add sand everywhere")) {
			for(int i = 0; i < width; ++i) {
				for(int j = 0; j < height; ++j) {
					sp.addSand(i, j, quantity);
				}
			}
		}
		if (ImGui::Button("Reset grid")) {
			sp.reset();
		}

		if (!worker_running && ImGui::Button("Force to update everything")) {
			sp.markGridToUpdate();
		}
		ImGui::PopItemWidth();
	}

	if (ImGui::CollapsingHeader("Rule")) {
		constexpr int max_size_rule = 7;
		static int rule_width = 1, rule_height = 1;
		static grid rule { { 0, 1, 0 }, { 1, 0, 1 }, { 0, 1, 0 } };


		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::SliderInt("rule - half width", &rule_width, 1, max_size_rule);
		ImGui::SliderInt("rule - half height", &rule_height, 1, max_size_rule);
		ImGui::PopItemWidth();

		rule.resize(2 * rule_width + 1, std::vector<uint64_t>(2 * rule_height + 1, 0));
		for(auto& row : rule)
			row.resize(2 * rule_height + 1, 0);
		rule[rule_width][rule_height] = 0;

		ImGui::PushItemWidth(40);
		for(int j = 0; j < 2 * rule_height + 1; ++j) {
			for(int i = 0; i < 2 * rule_width + 1; ++i) {
				ImGui::PushID(i * (2 * max_size_rule + 1) + j);
				if (i == rule_width && j == rule_height)
					ImGui::Dummy(ImVec2(40, 20));
				else
					ImGui::InputScalar("", ImGuiDataType_U64, &rule[i][j]);
				ImGui::PopID();
				if (i < (2 * rule_width + 1) - 1) ImGui::SameLine();
			}
		}
		ImGui::PopItemWidth();

		if (!worker_running && ImGui::Button("Change expansion rule")) {
			sp.setExpansionRule(rule);
		}
	}

	if (ImGui::CollapsingHeader("Worker status")) {
		std::string running = worker_running ? "Worker is running" : "Worker is not running";
		ImGui::Text(running.c_str());

		std::string nb_cells = std::to_string(sp.cellsToUpdate()) + " cells still waiting for update";
		ImGui::Text(nb_cells.c_str());
	}

}

int main() {
	std::thread worker(sandpile_loop);

	bool running = true;
	sf::RenderWindow window(sf::VideoMode(720, 720), "Sandpiles");
	ImGui::SFML::Init(window);
	ImGui::GetIO().FontGlobalScale = 1.5f;

	sf::Texture texture;
	texture.create(width, height);

	try {
		sf::Clock delta;
		while (running) {
			sf::Event event;
			while (window.pollEvent(event)) {
				ImGui::SFML::ProcessEvent(event);
				running = event.type != event.Closed;
			}

			window.clear();

			auto pixels = sp.getGrid();
			texture.update(pixels.data());

			sf::Sprite sprite(texture);
			auto [w_w, w_h] = window.getView().getSize();
			sprite.setScale(w_w / width, w_h / height);
			window.draw(sprite);

			ImGui::SFML::Update(window, delta.restart());
			showSandpileControl();
			showDebugConsole();
			ImGui::SFML::Render(window);

			window.display();
		}

		ImGui::SFML::Shutdown();
		window.close();
	}
	catch (std::exception& e) {
		continue_sim = false;
		worker.join();
		logError(e.what());
		std::cout << e.what() << '\n';
	}
	return 0;
}