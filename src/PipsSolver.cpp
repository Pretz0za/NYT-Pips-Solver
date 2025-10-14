#include "pips/PipsSolver.hpp"
#include <initializer_list>
#include <numeric>
#include <set>
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
