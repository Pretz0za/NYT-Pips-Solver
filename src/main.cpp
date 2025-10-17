#include "examples/easyPreFilledState.cpp"
#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  using namespace EasyPrefillExample;

  std::cout << "Starting solve" << '\n';
  Solver.solve();
  std::cout << "Finished solve" << '\n';
  return 0;
}
