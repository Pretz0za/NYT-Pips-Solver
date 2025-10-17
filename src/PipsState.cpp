#include "pips/PipsState.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

// Domino Class ----------------------------------------------------------------

Domino::Domino(std::pair<int, int> value) : value(value) {
  this->position = {-1, -1};
  this->orientation = std::pair{false, false};
}

void Domino::setPosition(const Position &newPosition) {
  this->position = newPosition;
}

void Domino::setOrientation(const std::pair<bool, bool> &newOrientation) {
  this->orientation = newOrientation;
}

void Domino::rotate() {
  orientation.second = !orientation.second;
  position.x += 1;
}

void Domino::swap(Domino &other) {
  std::swap(position, other.position);
  std::swap(orientation, other.orientation);
}

std::pair<Position, Position> Domino::getPosition() const {
  if (this->orientation.first == false) {
    return {this->position,
            {this->position.x + (this->orientation.second ? -1 : 1),
             this->position.y}};
  } else
    return {this->position,
            {this->position.x,
             this->position.y + (this->orientation.second ? -1 : 1)}};
}

std::pair<int, int> Domino::getValues() const { return this->value; }

// End Domino Class ------------------------------------------------------------

// Tile Class ------------------------------------------------------------------

Tile::Tile(int value) : value(value) {}

int Tile::getValue() const { return this->value; }

void Tile::setValue(int newValue) { this->value = newValue; }

// End Tile Class --------------------------------------------------------------

// Constraint Classes ----------------------------------------------------------

Constraint::Constraint(std::vector<Position> tiles) : tiles(std::move(tiles)) {}
std::vector<Position> Constraint::getTiles() const { return tiles; }
int Constraint::evaluate(std::initializer_list<int> values) const { return 0; }

EqualConstraint::EqualConstraint(std::vector<Position> tiles)
    : Constraint(std::move(tiles)) {}
int EqualConstraint::evaluate(std::initializer_list<int> values) const {
  const int mode = findMode(values);
  int count = 0;
  for (const int &curr : values) {
    if (curr != mode)
      count++;
  }
  return count;
}

UniqueConstraint::UniqueConstraint(std::vector<Position> tiles)
    : Constraint(std::move(tiles)) {}
int UniqueConstraint::evaluate(std::initializer_list<int> values) const {
  return findRepeatCount(values);
}

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
int LessThanConstraint::evaluate(std::initializer_list<int> values) const {
  int sum = std::accumulate(values.begin(), values.end(), 0);
  if (sum < limit)
    return 0;
  return (sum - limit + 2) / 3; // +2 to round up instead of down
}

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
                                             int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
int GreaterThanConstraint::evaluate(std::initializer_list<int> values) const {
  int sum = std::accumulate(values.begin(), values.end(), 0);
  if (sum > limit)
    return 0;
  return (sum - limit + 2) / 3;
}

ExactSumConstraint::ExactSumConstraint(std::vector<Position> tiles, int target)
    : Constraint(std::move(tiles)), target(target) {}
int ExactSumConstraint::evaluate(std::initializer_list<int> values) const {
  return abs(std::accumulate(values.begin(), values.end(), 0) - target + 2) / 3;
}

// End Constraint Classes ------------------------------------------------------

// Grid Class ------------------------------------------------------------------

template <int Width, int Height>

Grid<Width, Height>::Grid(const std::vector<Position> &disabledTiles) {
  for (const auto &pos : disabledTiles) {
    if (pos.x >= Width || pos.y >= Height)
      throw std::runtime_error("Disabled tiles out of bounds");
    *this[pos] = -2;
  }
}

template <int Width, int Height>
Tile &Grid<Width, Height>::operator[](const Position &pos) {
  if (pos.x >= Width || pos.y >= Height)
    throw std::runtime_error("Indexed position out of bounds");
  return grid[pos.x][pos.y];
}

template <int Width, int Height>
const Tile &Grid<Width, Height>::operator[](const Position &pos) const {
  if (pos.x >= Width || pos.y >= Height)
    throw std::runtime_error("Indexed position out of bounds");
  return grid[pos.x][pos.y];
}

template <int Width, int Height>
std::array<std::array<Tile, Width>, Height>
Grid<Width, Height>::getGrid() const {
  return grid;
}

// End Grid Class --------------------------------------------------------------

// PipsState Class -------------------------------------------------------------

template <int Width, int Height>
PipsState<Width, Height>::PipsState(
    std::vector<Position> disabledTiles, std::vector<Domino> dominos,
    std::shared_ptr<std::vector<Constraint>> constraints)
    : constraints(std::move(constraints)), dominos(std::move(dominos)),
      grid(disabledTiles) {}

template <int Width, int Height>
void PipsState<Width, Height>::rotateDomino(int index) {
  dominos[index].rotate();
  const auto [pos1, pos2] = dominos[index].getPosition();
  std::swap(grid[pos1.x][pos1.y], grid[pos2.x][pos2.y]);
}

template <int Width, int Height>
void PipsState<Width, Height>::placeDomino(const Domino &domino) {
  auto [pos1, pos2] = domino.getPosition();
  auto [val1, val2] = domino.getValues();
  grid[pos1.x][pos1.y] = val1;
  grid[pos2.x][pos2.y] = val2;
}

template <int Width, int Height>
void PipsState<Width, Height>::swapDominos(int first, int second) {
  dominos[first].swap(dominos[second]);
  placeDomino(dominos[first]);
  placeDomino(dominos[second]);
}

template <int Width, int Height>
std::initializer_list<int> PipsState<Width, Height>::getValues(
    const std::vector<Position> &positions) const {
  std::initializer_list<int> values{};
  std::transform(positions.begin(), positions.end(), values.begin(),
                 [&](const Position &pos) { return grid[pos]; });
}

template <int Width, int Height>
bool PipsState<Width, Height>::isSolved() const {
  for (const auto &constraint : *constraints) {
    std::initializer_list<int> values{};
    std::vector<Position> positions = constraint.getTiles();
    std::transform(
        positions.begin(), positions.end(), values.begin(),
        [&](Position &position) { return grid[position.x][position.y]; });
    if (constraint.evaluate(values) != 0)
      return false;
  }
  return true;
}

template <int Width, int Height>
std::vector<PipsAction> PipsState<Width, Height>::availableActions() const {
  std::vector<PipsAction> output{};
  for (int i = 0; i < dominos.size(); i++) {
    output.push_back({PipsActionType::Rotate, i, i});
    for (int j = i + 1; j < dominos.size(); j++) {
      output.push_back({PipsActionType::Swap, i, j});
    }
  }
  return output;
}

template <int Width, int Height>
PipsState<Width, Height>
PipsState<Width, Height>::stateAfterAction(const PipsAction &action) const {
  PipsState output{*this};
  if (action.action == PipsActionType::Swap) {
    output.swapDominos(action.first, action.second);
  } else if (action.action == PipsActionType::Rotate) {
    output.rotateDomino(action.first);
  }
  return output;
}

template <int Width, int Height>
int PipsState<Width, Height>::objective() const {
  int error = 0;
  for (const auto &constraint : *constraints) {
    error += constraint.evaluate(getValues(constraint.getTiles()));
  }
  return error;
}

// End PipsState Class ---------------------------------------------------------
