#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace Feb04HardPips {

std::vector<Position> disabledTiles{
	{0, 0}, {0, 1}, {0, 3}, {0, 4}, {1, 1}, {1, 3}, {2, 1}, {2, 3},
	{4, 0}, {4, 1}, {4, 3}, {4, 4}, {5, 0}, {5, 1}, {5, 3}, {5, 4}};

auto constraints = std::vector<std::shared_ptr<Constraint>>{

	std::make_shared<GreaterThanConstraint>(
		std::initializer_list<Position>{{1, 0}}, 2),

	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 0}, {3, 0}}, 7),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 2}, {2, 2}}, 7),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 4}, {2, 4}, {3, 4}}, 7),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{3, 2}, {4, 2}}, 7),
	std::make_shared<GreaterThanConstraint>(
		std::initializer_list<Position>{{3, 3}}, 1),
	std::make_shared<GreaterThanConstraint>(
		std::initializer_list<Position>{{5, 2}}, 0),

};

auto dominos = std::vector<std::shared_ptr<Domino>>{
	std::make_shared<Domino>(std::pair<int, int>{0, 1}),
	std::make_shared<Domino>(std::pair<int, int>{4, 0}),
	std::make_shared<Domino>(std::pair<int, int>{4, 3}),
	std::make_shared<Domino>(std::pair<int, int>{1, 4}),
	std::make_shared<Domino>(std::pair<int, int>{6, 3}),
	std::make_shared<Domino>(std::pair<int, int>{2, 1}),
	std::make_shared<Domino>(std::pair<int, int>{5, 1}),
};

PipsAI<5, 6> Solver{disabledTiles, dominos, constraints};

} // namespace Feb04HardPips
