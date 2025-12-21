#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace Dec21MedPips {

std::vector<Position> disabledTiles{{2, 4}};

auto constraints = std::vector<std::shared_ptr<Constraint>>{

	std::make_shared<EqualConstraint>(
		std::initializer_list<Position>{{0, 0}, {0, 1}, {1, 0}}),
	std::make_shared<EqualConstraint>(
		std::initializer_list<Position>{{0, 2}, {0, 3}}),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 1}, {2, 1}}, 6),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{1, 2}, {1, 3}, {1, 4}}, 11),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 0}}, 6),
	std::make_shared<ExactSumConstraint>(
		std::initializer_list<Position>{{2, 2}}, 6),
	std::make_shared<LessThanConstraint>(
		std::initializer_list<Position>{{2, 3}}, 4),

};

auto dominos = std::vector<std::shared_ptr<Domino>>{
	std::make_shared<Domino>(std::pair<int, int>{5, 5}),
	std::make_shared<Domino>(std::pair<int, int>{0, 1}),
	std::make_shared<Domino>(std::pair<int, int>{1, 4}),
	std::make_shared<Domino>(std::pair<int, int>{3, 6}),
	std::make_shared<Domino>(std::pair<int, int>{5, 6}),
	std::make_shared<Domino>(std::pair<int, int>{2, 6}),
	std::make_shared<Domino>(std::pair<int, int>{2, 2}),
};

PipsAI<5, 3> Solver{disabledTiles, dominos, constraints};

}; // namespace Dec21MedPips
