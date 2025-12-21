#include "examples/easyPreFilledState.cpp"
#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <iostream>
#include <optional>

int main(int argc, char *argv[]) {
	using namespace EasyPrefillExample;
	if (auto solution = Solver.solve()) {
		solution->printSolution();
	} else {
		std::cout << "No solution found\n";
	}
	return 0;
}
