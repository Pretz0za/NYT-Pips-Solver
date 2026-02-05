#include "examples/dec21HardPips.cpp"
#include "examples/dec21MedPips.cpp"
#include "examples/feb04HardPips.cpp"
#include "examples/nov19HardPips.cpp"
#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <iostream>
#include <optional>

int main(int argc, char *argv[]) {

	if (auto solution = Feb04HardPips::Solver.solve()) {
		solution->printBoard("Feb. 04 2026 NYT Pips Hard. Solved.");
	} else {
		std::cout << "No solution found\n";
	}

	if (auto solution = Dec21HardPips::Solver.solve()) {
		solution->printBoard("Dec. 21 2026 NYT Pips Hard. Solved.");
	} else {
		std::cout << "No solution found\n";
	}

	return 0;
}
