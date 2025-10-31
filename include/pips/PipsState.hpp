#ifndef PIPS_STATE_HPP
#define PIPS_STATE_HPP

#include <algorithm>
#include <array>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// Declarations

struct Position {
	int row;
	int col;
};

namespace Orientation {
inline std::pair Right{false, false};
inline std::pair Left{false, true};
inline std::pair Down{true, false};
inline std::pair Up{true, true};
} // namespace Orientation

class Domino {
	std::pair<int, int> value;

  public:
	Domino(std::pair<int, int> value);

	// Returns the values of the domino
	std::pair<int, int> getValues() const;

	~Domino() = default;
};

class Tile {
	int value;
	std::shared_ptr<Domino> domino;
	std::pair<bool, bool> orientation;

  public:
	Tile(int value = 0, std::shared_ptr<Domino> domino = nullptr,
		 std::pair<bool, bool> orientation = {false, false});

	// gets/sets the value of the tile and errors if the tile is out of play
	int getValue() const;
	void setValue(int newValue);

	std::shared_ptr<Domino> getDomino() const;
	void setDomino(std::shared_ptr<Domino> newDomino);

	std::pair<bool, bool> getOrientation() const;
	void setOrientation(std::pair<bool, bool> newOrientation);

	~Tile() = default;
};

template <int Width, int Height> class Grid {
	std::array<std::array<Tile, Width>, Height> grid;

  public:
	Grid();
	Grid(const std::vector<Position> &disabledTiles);
	std::array<std::array<Tile, Width>, Height> getGrid() const;

	Tile &operator[](const Position &pos);
	const Tile &operator[](const Position &pos) const;

	~Grid() = default;
};

class Constraint {
  protected:
	std::vector<Position> positions;

  public:
	Constraint(std::vector<Position> tiles);

	std::vector<Position> getPositions() const;
	virtual bool evaluate(std::vector<int> values) const = 0;

	virtual ~Constraint() = default;
};

class EqualConstraint : public Constraint {
  public:
	EqualConstraint(std::vector<Position> tiles);
	bool evaluate(std::vector<int> values) const override;
	~EqualConstraint() = default;
};

class UniqueConstraint : public Constraint {
  public:
	UniqueConstraint(std::vector<Position> tiles);
	bool evaluate(std::vector<int> values) const override;
	~UniqueConstraint() = default;
};

class LessThanConstraint : public Constraint {
	int limit;

  public:
	LessThanConstraint(std::vector<Position> tiles, int limit);
	bool evaluate(std::vector<int> values) const override;
	~LessThanConstraint() = default;
};

class GreaterThanConstraint : public Constraint {
	int limit;

  public:
	GreaterThanConstraint(std::vector<Position> tiles, int limit);
	bool evaluate(std::vector<int> values) const override;
	~GreaterThanConstraint() = default;
};

class ExactSumConstraint : public Constraint {
	int target;

  public:
	ExactSumConstraint(std::vector<Position> tiles, int target);
	bool evaluate(std::vector<int> values) const override;
	~ExactSumConstraint() = default;
};

enum PipsActionType { Rotate, Swap };

struct PipsAction {
	PipsActionType action;
	int first;
	int second;
};

template <int Width, int Height> class PipsSolver;

template <int Width, int Height> class PipsState {
	Grid<Width, Height> grid;
	std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos;
	std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints;

	void placeDomino(std::shared_ptr<Domino> domino, Position position,
					 std::pair<bool, bool> orientation);

  public:
	PipsState(
		std::vector<Position> disabledTiles,
		std::shared_ptr<std::vector<std::shared_ptr<Domino>>> dominos,
		std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints);
	PipsState(PipsState<Width, Height> &&other);

	PipsState(const PipsState<Width, Height> &other);
	PipsState<Width, Height> &operator=(const PipsState<Width, Height> &other);

	std::vector<Tile> getTiles(const std::vector<Position> &positions) const;

	bool isSolved() const;

	~PipsState() = default;

	friend PipsSolver<Width, Height>;
};

// Definitions

// Grid Class ------------------------------------------------------------------

template <int Width, int Height>

Grid<Width, Height>::Grid() : grid{{Tile()}} {}

template <int Width, int Height>
Grid<Width, Height>::Grid(const std::vector<Position> &disabledTiles)
	: grid{{Tile()}} {

	for (const auto &pos : disabledTiles) {
		if (pos.row >= Width || pos.col >= Height)
			throw std::runtime_error("Disabled tiles out of bounds");
		grid[pos.row][pos.col].setValue(-1);
	}
}

template <int Width, int Height>
Tile &Grid<Width, Height>::operator[](const Position &pos) {
	if (pos.row >= Height || pos.col >= Width) {
		throw std::runtime_error("Indexed position out of bounds");
	}
	return grid[pos.row][pos.col];
}

template <int Width, int Height>
const Tile &Grid<Width, Height>::operator[](const Position &pos) const {
	if (pos.row >= Height || pos.col >= Width) {
		throw std::runtime_error("Indexed position out of bounds");
	}
	return grid[pos.row][pos.col];
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
	std::vector<Position> disabledTiles,
	std::shared_ptr<std::vector<std::shared_ptr<Domino>>> dominos,
	std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints)
	: constraints(std::move(constraints)), dominos(std::move(dominos)),
	  grid(disabledTiles) {}

template <int Width, int Height>
PipsState<Width, Height>::PipsState(PipsState<Width, Height> &&other)
	: constraints(std::move(other.constraints)),
	  dominos(std::move(other.dominos)), grid(std::move(other.grid)) {}

template <int Width, int Height>
PipsState<Width, Height>::PipsState(const PipsState &other)
	: dominos(other.dominos), constraints(other.constraints), grid(other.grid) {
}

template <int Width, int Height>
PipsState<Width, Height> &
PipsState<Width, Height>::operator=(const PipsState &other) {
	constraints = other.constraints;
	dominos = other.dominos;
	grid = other.grid;
	return *this;
}

template <int Width, int Height>
void PipsState<Width, Height>::placeDomino(std::shared_ptr<Domino> domino,
										   Position position,
										   std::pair<bool, bool> orientation) {
	auto [val1, val2] = domino->getValues();
	grid[position].setValue(val1);
	if (orientation.first) {
		if (orientation.second)
			grid[position.row - 1][position.col].setValue(val2);
		else
			grid[position.row + 1][position.col].setValue(val2);
	} else {
		if (orientation.second)
			grid[position.row][position.col - 1].setValue(val2);
		else
			grid[position.row][position.col + 1].setValue(val2);
	}
}

template <int Width, int Height>
std::vector<Tile> PipsState<Width, Height>::getTiles(
	const std::vector<Position> &positions) const {
	std::vector<Tile> values{static_cast<int>(positions.size())};
	std::transform(positions.begin(), positions.end(), values.begin(),
				   [&](const Position &pos) { return grid[pos]; });
	return values;
}

template <int Width, int Height>
bool PipsState<Width, Height>::isSolved() const {
	std::cout << "Entering isSolved()" << '\n';
	std::cout << "Constraints: " << constraints << '\n'; // FIX: NULLPTR!!
	for (const auto &row : grid) {
		for (const auto &tile : row) {
			if (!tile.getDomino() && tile.getValue != -1)
				return false;
		}
	}
	for (const auto &constraint : *constraints) {
		std::cout << "ITS FIXED" << '\n';
		std::vector<Position> positions = constraint->getPositions();
		std::vector<int> values{static_cast<int>(positions.size())};
		std::transform(positions.begin(), positions.end(), values.begin(),
					   [&](Position &position) {
						   Tile tile = grid[position];
						   if (tile.getValue() == -1)
							   throw std::runtime_error(
								   "isSolved: checking disabled tile against "
								   "constraint is illegal");
						   return tile.getValue();
					   });
		std::cout << "ITS FIXED" << '\n';
		if (!constraint->evaluate(values))
			return false;
	}

	return true;
}

#endif
