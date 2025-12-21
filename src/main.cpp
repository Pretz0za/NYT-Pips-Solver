#include "examples/dec21HardPips.cpp"
#include "examples/dec21MedPips.cpp"
#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <iostream>
#include <optional>

int main(int argc, char *argv[]) {
	// std::vector<Tile> tiles;
	// auto d1 = std::make_shared<Domino>(std::pair<int, int>{1, 1});
	// auto d2 = std::make_shared<Domino>(std::pair<int, int>{6, 3});
	//
	// tiles.push_back(Tile(true, d1, Orientation::Down, true));
	// tiles.push_back(Tile(true, d1, Orientation::Up, true));
	// int x = 20;
	// int y = 10;
	// std::cout << "\x1b[2J\x1b[H";
	// for (const auto &tile : tiles) {
	// 	std::cout << "\x1b[" << y << ";" << x + 1 << "H";
	// 	tile.printTile(x, y);
	// 	y += 5;
	// }
	// x = 28;
	// y = 10;
	// Tile(true, d2, Orientation::Right, true).printTile(x, y);
	if (auto solution = Dec21MedPips::Solver.solve()) {
		solution->printSolution("Dec. 21 2025 NYT Pips Medium. Solved.");
	} else {
		std::cout << "No solution found\n";
	}

	if (auto solution = Dec21HardPips::Solver.solve()) {
		solution->printSolution("Dec. 21 2025 NYT Pips Hard. Solved.");
	} else {
		std::cout << "No solution found\n";
	}

	return 0;
}
