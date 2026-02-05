#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace Nov19HardPips {

std::vector<Position> disabledTiles{{0, 0}, {0, 3}, {1, 3}, {1, 5}, {1, 6},
									{2, 3}, {2, 5}, {2, 6}, {3, 3}, {3, 5},
									{3, 6}, {3, 0}, {4, 0}, {4, 3}};

auto constraints = std::vector<std::shared_ptr<Constraint>>{

	std::make_shared<EqualConstraint>(std::initializer_list<Position>{
		{0, 1}, {1, 0}, {1, 1}, {2, 1}, {3, 1}, {4, 1}}),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{0, 2}, {1, 2}, {2, 2}}, 10),

	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{3, 2}, {4, 2}}, 10),

	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{4, 4}, {4, 5}}, 10),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{4, 6}, {4, 7}}, 10),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 7}, {3, 7}}, 10),

	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{0, 4}, {0, 5}, {0, 6}, {1, 4}}, 10),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 4}, {3, 4}}, 10),
	std::make_shared<EqualConstraint>(
		std::initializer_list<Position>{{0, 7}, {1, 7}}),
};

auto dominos = std::vector<std::shared_ptr<Domino>>{
	std::make_shared<Domino>(std::pair<int, int>{4, 3}),
	std::make_shared<Domino>(std::pair<int, int>{2, 0}),
	std::make_shared<Domino>(std::pair<int, int>{4, 5}),
	std::make_shared<Domino>(std::pair<int, int>{0, 0}),
	std::make_shared<Domino>(std::pair<int, int>{3, 5}),
	std::make_shared<Domino>(std::pair<int, int>{6, 6}),
	std::make_shared<Domino>(std::pair<int, int>{0, 1}),

	std::make_shared<Domino>(std::pair<int, int>{5, 5}),
	std::make_shared<Domino>(std::pair<int, int>{0, 6}),
	std::make_shared<Domino>(std::pair<int, int>{1, 4}),
	std::make_shared<Domino>(std::pair<int, int>{2, 2}),
	std::make_shared<Domino>(std::pair<int, int>{5, 0}),
	std::make_shared<Domino>(std::pair<int, int>{3, 1}),
};

PipsAI<8, 5> Solver{disabledTiles, dominos, constraints};

} // namespace Nov19HardPips
