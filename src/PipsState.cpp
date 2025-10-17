#include "pips/PipsState.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <utility>
#include <vector>

// Domino Class ----------------------------------------------------------------

Domino::Domino(std::pair<int, int> value) : value(value) {
  this->position = {-1, -1};
  this->orientation = std::pair{false, false};
}

Domino::Domino(std::pair<int, int> value, Position position,
               std::pair<bool, bool> orientation)
    : value(value), position(position), orientation(orientation) {}

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
  // first = 0 -> left to right
  // first = 1 -> up to down.
  // second = 0 -> nothing.
  // second = 1 -> reverse (right to left, down to up)

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
int Constraint::evaluate(std::vector<int> values) const { return 0; }

EqualConstraint::EqualConstraint(std::vector<Position> tiles)
    : Constraint(std::move(tiles)) {}
int EqualConstraint::evaluate(std::vector<int> values) const {
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
int UniqueConstraint::evaluate(std::vector<int> values) const {
  return findRepeatCount(values);
}

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
int LessThanConstraint::evaluate(std::vector<int> values) const {
  int sum = std::accumulate(values.begin(), values.end(), 0);
  if (sum < limit)
    return 0;
  return (sum - limit + 2) / 3; // +2 to round up instead of down
}

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
                                             int limit)
    : Constraint(std::move(tiles)), limit(limit) {}
int GreaterThanConstraint::evaluate(std::vector<int> values) const {
  int sum = std::accumulate(values.begin(), values.end(), 0);
  if (sum > limit)
    return 0;
  return (sum - limit + 2) / 3;
}

ExactSumConstraint::ExactSumConstraint(std::vector<Position> tiles, int target)
    : Constraint(std::move(tiles)), target(target) {}
int ExactSumConstraint::evaluate(std::vector<int> values) const {
  return abs(std::accumulate(values.begin(), values.end(), 0) - target + 2) / 3;
}

// End Constraint Classes ------------------------------------------------------
