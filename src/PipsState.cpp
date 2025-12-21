#include "pips/PipsState.hpp"
#include "helpers.hpp"
#include "pips/PipsSolver.hpp"
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

Tile::Tile(bool inPlay)
	: side{}, domino{nullptr}, orientation{}, active(inPlay) {}

Tile::Tile(bool side, std::shared_ptr<Domino> domino,
		   std::pair<bool, bool> orientation, bool inPlay)
	: side(side), domino(std::move(domino)),
	  orientation(std::move(orientation)), active(inPlay) {}

Tile &Tile::operator=(const Tile &other) {
	active = other.active;
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

void Tile::printTile(int x, int y) const {

	if (orientation == Orientation::Left) {
		switch (getValue()) {
		case 0:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, "--------");
			break;
		case 1:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, "--------");
			break;
		case 2:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "|     x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x     |");
			printAt(x, y + 4, "--------");
			break;
		case 3:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "|     x |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "| x     |");
			printAt(x, y + 4, "--------");
			break;
		case 4:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, "--------");
			break;
		case 5:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, "--------");
			break;
		case 6:
			printAt(x, y + 0, "--------");
			printAt(x, y + 1, "| x x x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x x x |");
			printAt(x, y + 4, "--------");
			break;
		}
	} else if (orientation == Orientation::Right) {
		switch (getValue()) {
		case 0:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, " --------");
			break;
		case 1:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, " --------");
			break;
		case 2:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "| x     |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|     x |");
			printAt(x, y + 4, " --------");
			break;
		case 3:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "| x     |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|     x |");
			printAt(x, y + 4, " --------");
			break;
		case 4:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, " --------");
			break;
		case 5:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "| x   x ");
			printAt(x, y + 2, "|   x   ");
			printAt(x, y + 3, "| x   x ");
			printAt(x, y + 4, " --------");
			break;
		case 6:
			printAt(x, y + 0, " --------");
			printAt(x, y + 1, "| x x x ");
			printAt(x, y + 2, "|       ");
			printAt(x, y + 3, "| x x x ");
			printAt(x, y + 4, " --------");
			break;
		}
	} else if (orientation == Orientation::Up) {

		switch (getValue()) {
		case 0:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, " -------");
			break;

		case 1:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, " -------");
			break;

		case 2:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "|     x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x     |");
			printAt(x, y + 4, " -------");
			break;

		case 3:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "|     x |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "| x     |");
			printAt(x, y + 4, " -------");
			break;

		case 4:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, " -------");
			break;

		case 5:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, " -------");
			break;

		case 6:
			printAt(x, y + 0, "| ----- |");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "| x   x |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, " -------");
			break;
		}
	} else {
		switch (getValue()) {
		case 0:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 1:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "|       |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|       |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 2:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "| x     |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "|     x |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 3:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "| x     |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "|     x |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 4:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|       |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 5:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "|   x   |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, "| ----- |");
			break;

		case 6:
			printAt(x, y + 0, " -------");
			printAt(x, y + 1, "| x   x |");
			printAt(x, y + 2, "| x   x |");
			printAt(x, y + 3, "| x   x |");
			printAt(x, y + 4, "| ----- |");
			break;
		}
	}
}

// End Tile Class --------------------------------------------------------------

// Constraint Classes ----------------------------------------------------------

Constraint::Constraint(std::vector<Position> tiles)
	: positions(std::move(tiles)) {}
std::vector<Position> Constraint::getPositions() const { return positions; }
bool Constraint::evaluate(std::span<int> values) const { return 0; }

EqualConstraint::EqualConstraint(std::vector<Position> tiles)
	: Constraint(std::move(tiles)) {}
bool EqualConstraint::evaluate(std::span<int> values) const {
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

bool EqualConstraint::evaluate(std::span<Tile> values) const {
	if (values.empty())
		return true;
	const auto first = values[0].getValue();
	int count = 0;
	for (const auto &curr : values) {
		if (curr.getValue() != first)
			return false;
	}
	return true;
}
bool EqualConstraint::evaluate(std::span<Variable> values) const {
	if (values.empty())
		return true;
	const auto first = values[0].getDomain().begin()->getValue();
	int count = 0;
	for (const auto &curr : values) {

		if (curr.getDomain().size() != 1)
			return false;

		if (curr.getDomain().begin()->getValue() != first)
			return false;
	}
	return true;
}

bool EqualConstraint::isBroken(std::vector<int> values) const {
	if (values.empty())
		return false;
	for (int val : values) {
		if (val != values[0])
			return true;
	}
	return false;
}

UniqueConstraint::UniqueConstraint(std::vector<Position> tiles)
	: Constraint(std::move(tiles)) {}
bool UniqueConstraint::evaluate(std::span<int> values) const {
	std::set<int> seen{};
	for (const auto &val : values) {
		if (seen.find(val) != seen.end())
			return false;
		seen.insert(val);
	}
	return true;
}

bool UniqueConstraint::evaluate(std::span<Tile> values) const {
	std::set<int> seen{};
	for (const auto &val : values) {
		if (seen.find(val.getValue()) != seen.end())
			return false;
		seen.insert(val.getValue());
	}
	return true;
}

bool UniqueConstraint::evaluate(std::span<Variable> values) const {
	std::set<int> seen{};
	for (const auto &val : values) {
		if (val.getDomain().size() != 1)
			return false;

		if (seen.find(val.getDomain().begin()->getValue()) != seen.end())
			return false;
		seen.insert(val.getDomain().begin()->getValue());
	}
	return true;
}

bool UniqueConstraint::isBroken(std::vector<int> values) const {
	std::set<int> seen;
	for (int val : values) {
		if (seen.find(val) != seen.end())
			return true;
		seen.insert(val);
	}
	return false;
}

LessThanConstraint::LessThanConstraint(std::vector<Position> tiles, int target)
	: Constraint(std::move(tiles)), target(target) {}
bool LessThanConstraint::evaluate(std::span<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum < target);
}

bool LessThanConstraint::evaluate(std::span<Tile> values) const {
	int sum = 0;
	for (const auto &val : values) {
		sum += val.getValue();
	}

	return (sum < target);
}

bool LessThanConstraint::evaluate(std::span<Variable> values) const {
	int sum = 0;
	for (const auto &val : values) {
		if (val.getDomain().size() != 1)
			return false;
		sum += val.getDomain().begin()->getValue();
	}
	return (sum < target);
}

int LessThanConstraint::getTarget() const { return target; }

bool LessThanConstraint::isBroken(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) >= target;
}

GreaterThanConstraint::GreaterThanConstraint(std::vector<Position> tiles,
											 int target)

	: Constraint(std::move(tiles)), target(target) {}
bool GreaterThanConstraint::evaluate(std::span<int> values) const {
	int sum = std::accumulate(values.begin(), values.end(), 0);
	return (sum > target);
}

bool GreaterThanConstraint::evaluate(std::span<Tile> values) const {
	int sum = 0;
	for (const auto &val : values) {
		sum += val.getValue();
	}
	return (sum > target);
}

bool GreaterThanConstraint::evaluate(std::span<Variable> values) const {
	int sum = 0;
	for (const auto &val : values) {
		if (val.getDomain().size() != 1)
			return false;
		sum += val.getDomain().begin()->getValue();
	}
	return (sum > target);
}

int GreaterThanConstraint::getTarget() const { return target; }

bool GreaterThanConstraint::isBroken(std::vector<int> values) const {
	int maxExtra = (positions.size() - values.size()) * 6;
	return std::accumulate(values.begin(), values.end(), maxExtra) <= target;
}

ExactSumConstraint::ExactSumConstraint(std::vector<Position> tiles, int target)
	: Constraint(std::move(tiles)), target(target) {}
bool ExactSumConstraint::evaluate(std::span<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) == target;
}

bool ExactSumConstraint::evaluate(std::span<Tile> values) const {
	int sum = 0;
	for (const auto &val : values) {
		sum += val.getValue();
	}
	return (sum == target);
}

bool ExactSumConstraint::evaluate(std::span<Variable> values) const {
	int sum = 0;
	for (const auto &val : values) {
		if (val.getDomain().size() != 1)
			return false;
		sum += val.getDomain().begin()->getValue();
	}
	return (sum == target);
}

int ExactSumConstraint::getTarget() const { return target; }

bool ExactSumConstraint::isBroken(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) > target;
}

// End Constraint Classes ------------------------------------------------------
