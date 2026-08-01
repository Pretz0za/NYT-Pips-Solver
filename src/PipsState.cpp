#include "pips/PipsState.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <map>
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
const std::vector<Position> &Constraint::getPositions() const {
	return positions;
}
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

std::vector<int> EqualConstraint::impossibleValues(
	const std::vector<std::shared_ptr<Domino>> &dominos) const {
	std::map<int, int> counts;
	for (const auto &domino : dominos) {
		counts[domino->getValues().first]++;
		counts[domino->getValues().second]++;
	}
	std::vector<int> output;
	for (const auto &pair : counts) {
		if (pair.second < positions.size()) {
			output.push_back(pair.first);
		}
	}
	return output;
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

bool UniqueConstraint::isBroken(std::vector<int> values) const {
	std::set<int> seen;
	for (int val : values) {
		if (seen.find(val) != seen.end())
			return true;
		seen.insert(val);
	}
	return false;
}

std::vector<int> UniqueConstraint::impossibleValues(
	const std::vector<std::shared_ptr<Domino>> &dominos) const {
	return {};
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

int LessThanConstraint::getTarget() const { return target; }

bool LessThanConstraint::isBroken(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) >= target;
}

std::vector<int> LessThanConstraint::impossibleValues(
	const std::vector<std::shared_ptr<Domino>> &dominos) const {
	std::vector<int> output;
	int zeroCount = 0;
	for (const auto &domino : dominos) {
		if (domino->getValues().first == 0)
			zeroCount++;
		if (domino->getValues().second == 0)
			zeroCount++;
	}
	int nonZero = positions.size() - zeroCount - 1;

	for (int i = 0; i < 7; i++) {
		if (i >= target - std::min(nonZero, 0))
			output.push_back(i);
	}
	return output;
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

int GreaterThanConstraint::getTarget() const { return target; }

bool GreaterThanConstraint::isBroken(std::vector<int> values) const {
	int maxExtra = (positions.size() - values.size()) * 6;
	return std::accumulate(values.begin(), values.end(), maxExtra) <= target;
}

std::vector<int> GreaterThanConstraint::impossibleValues(
	const std::vector<std::shared_ptr<Domino>> &dominos) const {
	int min = target - 6 * (positions.size() - 1);
	std::vector<int> output;
	if (min >= 0)
		for (int i = 0; i <= min; i++) {
			output.push_back(i);
		}
	return output;
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

int ExactSumConstraint::getTarget() const { return target; }

bool ExactSumConstraint::isBroken(std::vector<int> values) const {
	return std::accumulate(values.begin(), values.end(), 0) > target;
}

std::vector<int> ExactSumConstraint::impossibleValues(
	const std::vector<std::shared_ptr<Domino>> &dominos) const {
	int min = target - 6 * (positions.size() - 1);
	std::vector<int> output;
	if (min >= 0)
		for (int i = 0; i < min; i++) {
			output.push_back(i);
		}
	return output;
}

// supportsValue implementations ----------------------------------------------
//
// valueMasks[i] holds, for scope position i, a uint8_t bitmask over pip values
// 0..6 of the values still available in that cell's domain. Each check below is
// SOUND: it returns false only when `value` at `positionIdx` cannot participate
// in ANY assignment satisfying the constraint in isolation.

namespace {
// Lowest pip value present in a (non-zero) mask.
inline int minVal(uint8_t m) { return std::countr_zero(m); }
// Highest pip value present in a (non-zero) mask.
inline int maxVal(uint8_t m) { return 7 - std::countl_zero(m); }
} // namespace

bool ExactSumConstraint::supportsValue(
	int value, std::size_t positionIdx,
	const std::vector<uint8_t> &valueMasks) const {
	int sumMin = 0, sumMax = 0;
	for (std::size_t i = 0; i < valueMasks.size(); i++) {
		if (i == positionIdx)
			continue;
		if (valueMasks[i] == 0)
			return false; // an other cell has no possible value
		sumMin += minVal(valueMasks[i]);
		sumMax += maxVal(valueMasks[i]);
	}
	return (sumMin + value <= target) && (sumMax + value >= target);
}

bool LessThanConstraint::supportsValue(
	int value, std::size_t positionIdx,
	const std::vector<uint8_t> &valueMasks) const {
	int sumMin = 0;
	for (std::size_t i = 0; i < valueMasks.size(); i++) {
		if (i == positionIdx)
			continue;
		if (valueMasks[i] == 0)
			return false;
		sumMin += minVal(valueMasks[i]);
	}
	return value + sumMin < target;
}

bool GreaterThanConstraint::supportsValue(
	int value, std::size_t positionIdx,
	const std::vector<uint8_t> &valueMasks) const {
	int sumMax = 0;
	for (std::size_t i = 0; i < valueMasks.size(); i++) {
		if (i == positionIdx)
			continue;
		if (valueMasks[i] == 0)
			return false;
		sumMax += maxVal(valueMasks[i]);
	}
	return value + sumMax > target;
}

bool EqualConstraint::supportsValue(
	int value, std::size_t positionIdx,
	const std::vector<uint8_t> &valueMasks) const {
	const uint8_t bit = static_cast<uint8_t>(1u << value);
	for (std::size_t i = 0; i < valueMasks.size(); i++) {
		if (i == positionIdx)
			continue;
		if (!(valueMasks[i] & bit))
			return false; // some other cell cannot take `value`
	}
	return true;
}

bool UniqueConstraint::supportsValue(
	int value, std::size_t positionIdx,
	const std::vector<uint8_t> &valueMasks) const {
	// After assigning `value` to positionIdx, the remaining cells must take
	// pairwise-distinct values, none equal to `value`. Check Hall's condition
	// over all subsets of the other cells.
	const uint8_t remove = static_cast<uint8_t>(~(1u << value));
	std::vector<uint8_t> others;
	others.reserve(valueMasks.size());
	for (std::size_t i = 0; i < valueMasks.size(); i++) {
		if (i == positionIdx)
			continue;
		others.push_back(static_cast<uint8_t>(valueMasks[i] & remove));
	}
	const int k = static_cast<int>(others.size());
	for (int subset = 1; subset < (1 << k); subset++) {
		uint8_t un = 0;
		int count = 0;
		for (int i = 0; i < k; i++) {
			if (subset & (1 << i)) {
				un |= others[i];
				count++;
			}
		}
		if (std::popcount(un) < count)
			return false; // Hall violated: fewer values than cells
	}
	return true;
}

// End Constraint Classes
// ------------------------------------------------------
