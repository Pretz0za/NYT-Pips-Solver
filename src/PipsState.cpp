#include "pips/PipsState.hpp"
#include <cstdlib>
#include <memory>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

// Domino Class ----------------------------------------------------------------

Domino::Domino(std::pair<int, int> value) : value(value) {}
std::pair<int, int> Domino::getValues() const { return this->value; }

// End Domino Class ------------------------------------------------------------

// Tile Class ------------------------------------------------------------------

Tile::Tile(int value, std::shared_ptr<Domino> domino,
		   std::pair<bool, bool> orientation)
	: value(value), domino(std::move(domino)),
	  orientation(std::move(orientation)) {}

int Tile::getValue() const { return value; }
void Tile::setValue(int newValue) { value = newValue; }

std::shared_ptr<Domino> Tile::getDomino() const { return domino; }
void Tile::setDomino(std::shared_ptr<Domino> newDomino) { domino = newDomino; }

std::pair<bool, bool> Tile::getOrientation() const { return orientation; }
void Tile::setOrientation(std::pair<bool, bool> newOrientation) {
	orientation = newOrientation;
}

// End Tile Class --------------------------------------------------------------

// Constraint Classes ----------------------------------------------------------

Constraint::Constraint(std::vector<Position> tiles)
	: positions(std::move(tiles)) {}
std::vector<Position> Constraint::getPositions() const { return positions; }
bool Constraint::evaluate(std::vector<int> values) const { return 0; }

EqualConstraint::EqualConstraint(std::vector<Position> tiles)
	: Constraint(std::move(tiles)) {}
bool EqualConstraint::evaluate(std::vector<int> values) const {
	if (values.empty())
		return true;
	const int first = values[0];
	int count = 0;
	for (const int &curr : values) {
		if (curr != first)
			return false;
	}
	return true;
}

UniqueConstraint::UniqueConstraint(std::vector<Position> tiles)
	: Constraint(std::move(tiles)) {}
bool UniqueConstraint::evaluate(std::vector<int> values) const {
	std::set<int> seen{};
	for (const auto &val : values) {
		if (seen.find(val) != seen.end())
			return false;
		seen.insert(val);
	}
	return true;
}

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int limit)
	: Constraint(std::move(tiles)), limit(limit) {}
bool LessThanConstraint::evaluate(std::vector<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum < limit);
}

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
											 int limit)
	: Constraint(std::move(tiles)), limit(limit) {}
bool GreaterThanConstraint::evaluate(std::vector<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum > limit);
}

ExactSumConstraint::ExactSumConstraint(std::vector<Position> tiles, int target)
	: Constraint(std::move(tiles)), target(target) {}
bool ExactSumConstraint::evaluate(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) == target;
}

// End Constraint Classes ------------------------------------------------------
