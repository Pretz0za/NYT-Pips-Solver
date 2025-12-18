#include "pips/PipsState.hpp"
#include "helpers.hpp"
#include <cstdlib>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

// Domino Class ----------------------------------------------------------------

Domino::Domino(std::pair<int, int> value) : value(value) {}
std::pair<int, int> Domino::getValues() const { return this->value; }

// End Domino Class ------------------------------------------------------------

// Tile Class ------------------------------------------------------------------

Tile::Tile(bool side, std::shared_ptr<Domino> domino,
		   std::pair<bool, bool> orientation, bool inPlay)
	: side(side), domino(std::move(domino)),
	  orientation(std::move(orientation)), inPlay(inPlay) {}

Tile &Tile::operator=(const Tile &other) {
	if (inPlay != other.inPlay)
		throw std::runtime_error("Cannot set value of inPlay to not inPlay");
	side = other.side;
	domino = other.domino;
	orientation = other.orientation;
	return *this;
}

void Tile::setState(bool newSide, std::shared_ptr<Domino> newDomino,
					std::pair<bool, bool> newOrientation) {
	side = newSide;
	domino = newDomino;
	orientation = newOrientation;
}

bool Tile::getSide() const { return side; }
void Tile::setSide(bool newSide) { side = newSide; }

std::shared_ptr<Domino> Tile::getDomino() const { return domino; }
void Tile::setDomino(std::shared_ptr<Domino> newDomino) {
	domino = std::move(newDomino);
}

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

bool EqualConstraint::isSolvable(
	std::map<Position, std::set<Tile>> &domains) const {
	for (auto &[variable, domain] : domains) {
		if (domain.size() == 1)
			continue;
		auto originalDomain = domain;
		domain.clear();
		auto newDomain = domain;
		for (auto value : originalDomain) {
			domain.insert(value);
			std::map<Position, std::set<Tile>> deleted =
				clearDominoFromDomains(domains, value.getDomino(), variable);
			if (isSolvable(domains))
				newDomain.insert(value);
			for (const auto &[variable, value] : deleted) {
				domains.at(variable).insert(value.begin(), value.end());
			}
		}

		if (newDomain.empty())
			return false;

		domain = newDomain;
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

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int target)
	: Constraint(std::move(tiles)), target(target) {}
bool LessThanConstraint::evaluate(std::vector<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum < target);
}
int LessThanConstraint::getTarget() const { return target; }

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
											 int target)
	: Constraint(std::move(tiles)), target(target) {}
bool GreaterThanConstraint::evaluate(std::vector<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum > target);
}

int GreaterThanConstraint::getTarget() const { return target; }

ExactSumConstraint::ExactSumConstraint(std::vector<Position> tiles, int target)
	: Constraint(std::move(tiles)), target(target) {}
bool ExactSumConstraint::evaluate(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) == target;
}
int ExactSumConstraint::getTarget() const { return target; }

// End Constraint Classes ------------------------------------------------------
