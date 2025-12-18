#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace EasyPrefillExample {

std::vector<Position> disabledTiles{{1, 1}, {1, 2}, {1, 3},
									{3, 0}, {3, 1}, {3, 2}};
auto constraints = std::vector<std::shared_ptr<Constraint>>{

	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{0, 1}, {0, 2}}, 6),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{0, 0}, {1, 0}}, 7),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 0}, {2, 1}}, 12),
	std::make_shared<EqualConstraint>(
		std::initializer_list<Position>{{2, 2}, {2, 3}, {3, 3}, {4, 3}}),
	std::make_shared<LessThanConstraint>(
		std::initializer_list<Position>{{4, 0}}, 3)};

auto dominos = std::vector<std::shared_ptr<Domino>>{
	std::make_shared<Domino>(std::pair<int, int>{5, 5}),
	std::make_shared<Domino>(std::pair<int, int>{6, 2}),
	std::make_shared<Domino>(std::pair<int, int>{4, 5}),
	std::make_shared<Domino>(std::pair<int, int>{0, 4}),
	std::make_shared<Domino>(std::pair<int, int>{6, 1}),
	std::make_shared<Domino>(std::pair<int, int>{6, 5}),
	std::make_shared<Domino>(std::pair<int, int>{4, 3}),
};

// Pips<4, 5> StartState{disabledTiles, dominos, constraints};
PipsAI<4, 5> Solver{disabledTiles, dominos, constraints};

}; // namespace EasyPrefillExample
