#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace Dec21HardPips {

std::vector<Position> disabledTiles{
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 6}, {1, 0}, {1, 1},
	{1, 2}, {2, 0}, {2, 6}, {3, 0}, {3, 2}, {3, 6}, {5, 5},
	{5, 6}, {6, 0}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}};

auto constraints = std::vector<std::shared_ptr<Constraint>>{

	std::make_shared<UniqueConstraint>(
		std::initializer_list<Position>{{0, 5}, {1, 4}, {1, 5}, {2, 5}}),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 3}}, 3),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 6}}, 2),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 1}, {3, 1}}, 11),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 2}, {2, 3}, {2, 4}}, 18),
	std::make_shared<EqualConstraint>(
		std::initializer_list<Position>{{3, 3}, {4, 3}}),
	std::make_shared<EqualConstraint>(std::initializer_list<Position>{
		{3, 4}, {3, 5}, {4, 4}, {4, 5}, {4, 6}}),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{4, 0}}, 5),
	std::make_shared<EqualConstraint>(std::initializer_list<Position>{
		{4, 1}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}}),
};

auto dominos = std::vector<std::shared_ptr<Domino>>{
	std::make_shared<Domino>(std::pair<int, int>{1, 1}),
	std::make_shared<Domino>(std::pair<int, int>{6, 3}),
	std::make_shared<Domino>(std::pair<int, int>{6, 4}),
	std::make_shared<Domino>(std::pair<int, int>{0, 1}),
	std::make_shared<Domino>(std::pair<int, int>{5, 4}),
	std::make_shared<Domino>(std::pair<int, int>{0, 5}),
	std::make_shared<Domino>(std::pair<int, int>{6, 1}),

	std::make_shared<Domino>(std::pair<int, int>{0, 0}),
	std::make_shared<Domino>(std::pair<int, int>{5, 6}),
	std::make_shared<Domino>(std::pair<int, int>{4, 2}),
	std::make_shared<Domino>(std::pair<int, int>{3, 4}),
	std::make_shared<Domino>(std::pair<int, int>{0, 6}),
	std::make_shared<Domino>(std::pair<int, int>{4, 1}),
	std::make_shared<Domino>(std::pair<int, int>{5, 1}),

};

PipsAI<7, 7> Solver{disabledTiles, dominos, constraints};

}; // namespace Dec21HardPips
