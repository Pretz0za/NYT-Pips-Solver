#include "pips/PipsSolver.hpp"
#include "pips/PipsState.hpp"
#include <initializer_list>
#include <memory>

namespace EasyPrefillExample {

std::vector<Position> disabledTiles{{1, 1}, {1, 2}, {1, 3},
                                    {3, 0}, {3, 1}, {3, 2}};
auto constraints = std::make_shared<std::vector<std::shared_ptr<Constraint>>>(
    std::initializer_list<std::shared_ptr<Constraint>>{
        std::make_shared<ExactSumConstraint>(
            std::initializer_list<Position>{{0, 1}, {0, 2}}, 6),
        std::make_shared<ExactSumConstraint>(
            std::initializer_list<Position>{{0, 0}, {1, 0}}, 7),
        std::make_shared<ExactSumConstraint>(
            std::initializer_list<Position>{{2, 0}, {2, 1}}, 12),
        std::make_shared<EqualConstraint>(
            std::initializer_list<Position>{{2, 2}, {2, 3}, {3, 3}, {4, 3}}),
        std::make_shared<LessThanConstraint>(
            std::initializer_list<Position>{{4, 0}}, 3)});

std::vector<Domino> dominos{
    {{5, 5}, {0, 3}, Orientation::Left}, {{6, 2}, {0, 1}, Orientation::Left},
    {{4, 5}, {1, 0}, Orientation::Down}, {{0, 4}, {2, 1}, Orientation::Right},
    {{6, 1}, {2, 3}, Orientation::Down}, {{6, 5}, {4, 3}, Orientation::Left},
    {{4, 3}, {4, 1}, Orientation::Left}};

PipsState<4, 5> StartState{disabledTiles, dominos, constraints};
PipsSolver Solver{StartState};

}; // namespace EasyPrefillExample
