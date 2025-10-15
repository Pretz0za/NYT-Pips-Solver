#include "pips/PipsSolver.hpp"
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

// PipsState Class -------------------------------------------------------------

PipsState::PipsState(int width, int height, std::vector<Position> disabledTiles,
                     std::vector<Domino> dominos,
                     std::shared_ptr<std::vector<Constraint>> constraints)
    : constraints(std::move(constraints)), dominos(std::move(dominos)),
      grid(height, std::vector<Tile>(width, Tile(-1))) {
  for (const auto &[x, y] : disabledTiles) {
    if (x >= width || y >= height)
      throw std::runtime_error("Disabled tiles out of bounds");
    grid[x][y] = -2;
  }
}

void PipsState::rotateDomino(int index) {
  dominos[index].rotate();
  const auto [pos1, pos2] = dominos[index].getPosition();
  std::swap(grid[pos1.x][pos1.y], grid[pos2.x][pos2.y]);
}

void PipsState::placeDomino(const Domino &domino) {
  auto [pos1, pos2] = domino.getPosition();
  auto [val1, val2] = domino.getValues();
  grid[pos1.x][pos1.y] = val1;
  grid[pos2.x][pos2.y] = val2;
}

void PipsState::swapDominos(int first, int second) {
  dominos[first].swap(dominos[second]);
	placeDomino(dominos[first]);
		placeDomino(dominos[second]);
}

bool PipsState::isSolved() const {
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
