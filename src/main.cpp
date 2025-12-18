#include "examples/easyPreFilledState.cpp"
#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	using namespace EasyPrefillExample;
	Solver.solve();
	std::cout << "Hello World!" << '\n';
	return 0;
}
