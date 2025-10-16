#include "pips/PipsState.hpp"
#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <set>
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
bool Constraint::evaluate(std::initializer_list<int> values) const {
  return true;
}

EqualConstraint::EqualConstraint(std::vector<Position> tiles)
    : Constraint(std::move(tiles)) {}
bool EqualConstraint::evaluate(std::initializer_list<int> values) const {
  const int *pFirst = nullptr;
  for (const int &curr : values) {
    if (!pFirst) {
      pFirst = &curr;
    } else {
      if (curr != *pFirst)
        return false;
    }
  }
  return true;
}

UniqueConstraint::UniqueConstraint(std::vector<Position> tiles)
    : Constraint(std::move(tiles)) {}
bool UniqueConstraint::evaluate(std::initializer_list<int> values) const {
  std::set<int> seen;
  for (const int &curr : values) {
    if (seen.find(curr) == seen.end()) {
      seen.insert(curr);
    } else
      return false;
  }
  return true;
}

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
bool LessThanConstraint::evaluate(std::initializer_list<int> values) const {
  return std::accumulate(values.begin(), values.end(), 0) < limit;
}

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
                                             int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
bool GreaterThanConstraint::evaluate(std::initializer_list<int> values) const {
  return std::accumulate(values.begin(), values.end(), 0) > limit;
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
std::array<std::array<Tile, Width>, Height> Grid<Width, Height>::getGrid() const {
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
bool PipsState<Width, Height>::isSolved() const {
  for (const auto &constraint : *constraints) {
    std::initializer_list<int> values{};
    std::vector<Position> positions = constraint.getTiles();
    std::transform(
        positions.begin(), positions.end(), values.begin(),
        [&](Position &position) { return grid[position.x][position.y]; });
    if (!constraint.evaluate(values))
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
  // Could do the following:
  //
  // Return how many TILES break each constraint (Not how many constraints are
  // broken). e.g. EqualConstraint would return the numbers of tiles that AREN'T
  // equal to whatever. LessThan and GreaterThan Constraints could estimate, get
  // the difference to the limit and divide by 3 (average of domino digits). We
  // drive this to 0.
}

// End PipsState Class ---------------------------------------------------------
