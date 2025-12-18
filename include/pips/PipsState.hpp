#ifndef PIPS_STATE_HPP
#define PIPS_STATE_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// Declarations

struct Position {
	int row;
	int col;

	bool operator==(const Position &) const = default;
};

namespace std {
template <> struct hash<Position> {
	std::size_t operator()(const Position &pos) const noexcept {
		std::size_t h = 0;
		h ^= std::hash<int>{}(pos.row) + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= std::hash<int>{}(pos.col) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};
} // namespace std

namespace Orientation {
inline std::pair Right{false, false};
inline std::pair Left{false, true};
inline std::pair Down{true, false};
inline std::pair Up{true, true};
} // namespace Orientation

template <int Width, int Height> class SolverState;
class Variable;

class Domino {
	std::pair<int, int> value;

  public:
	Domino(std::pair<int, int> value);

	int getValue(bool side) const { return side ? value.first : value.second; };
	std::pair<int, int> getValues() const;

	~Domino() = default;
};

class Tile {
	bool side;
	std::shared_ptr<Domino> domino;
	std::pair<bool, bool> orientation;

  public:
	const bool inPlay;
	Tile(bool inPlay = true);
	Tile(bool side, std::shared_ptr<Domino>, std::pair<bool, bool> orientation,
		 bool inPlay = true);

	Tile &operator=(const Tile &other);

	Tile getComplement() const {
		return Tile(!side, domino, {orientation.first, !orientation.second});
	}

	bool operator==(const Tile &other) const {
		return side == other.side && domino == other.domino &&
			   orientation == other.orientation;
	}

	void setState(bool newSide, std::shared_ptr<Domino> newDomino,
				  std::pair<bool, bool> newOrientation);

	int getValue() const { return domino->getValue(side); };

	bool getSide() const;
	void setSide(bool newSide);

	std::shared_ptr<Domino> getDomino() const;
	void setDomino(std::shared_ptr<Domino> newDomino);

	std::pair<bool, bool> getOrientation() const;
	void setOrientation(std::pair<bool, bool> newOrientation);

	~Tile() = default;
};

namespace std {
template <> struct hash<Tile> {
	std::size_t operator()(const Tile &tile) const {
		std::size_t h = 0;

		// hash side
		h ^= std::hash<bool>{}(tile.getSide()) + 0x9e3779b9 + (h << 6) +
			 (h >> 2);

		// hash domino pointer
		h ^= std::hash<Domino *>{}(tile.getDomino().get()) + 0x9e3779b9 +
			 (h << 6) + (h >> 2);

		// hash orientation
		auto o = tile.getOrientation();
		h ^= std::hash<bool>{}(o.first) + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= std::hash<bool>{}(o.second) + 0x9e3779b9 + (h << 6) + (h >> 2);

		return h;
	}
};
} // namespace std

template <int Width, int Height, typename K> class Grid {
	std::array<std::array<K, Width>, Height> grid;

  public:
	Grid();
	Grid(const std::vector<Position> &disabledTiles);
	std::array<std::array<K, Width>, Height> getGrid() const;

	K &operator[](const Position &pos);
	const K &operator[](const Position &pos) const;
	Grid<Width, Height, K> operator=(const Grid<Width, Height, K> &other) {
		grid = other.grid;
		return *this;
	}

	Position getOther(const Position &pos,
					  const std::pair<bool, bool> &orientaion) const;

	~Grid() = default;
};

class Constraint {
  protected:
	std::vector<Position> positions;

  public:
	Constraint(std::vector<Position> tiles);

	std::vector<Position> getPositions() const;
	virtual bool evaluate(std::span<int> values) const = 0;
	virtual bool evaluate(std::span<Variable> values) const = 0;
	virtual bool evaluate(std::span<Tile> values) const = 0;

	virtual ~Constraint() = default;
};

class EqualConstraint : public Constraint {
  public:
	EqualConstraint(std::vector<Position> tiles);
	virtual bool evaluate(std::span<int> values) const override;
	virtual bool evaluate(std::span<Variable> values) const override;
	virtual bool evaluate(std::span<Tile> values) const override;
	~EqualConstraint() = default;
};

class UniqueConstraint : public Constraint {
  public:
	UniqueConstraint(std::vector<Position> tiles);
	virtual bool evaluate(std::span<int> values) const override;
	virtual bool evaluate(std::span<Variable> values) const override;
	virtual bool evaluate(std::span<Tile> values) const override;
	~UniqueConstraint() = default;
};

class LessThanConstraint : public Constraint {
	int target;

  public:
	LessThanConstraint(std::vector<Position> tiles, int target);
	int getTarget() const;
	virtual bool evaluate(std::span<int> values) const override;
	virtual bool evaluate(std::span<Variable> values) const override;
	virtual bool evaluate(std::span<Tile> values) const override;
	~LessThanConstraint() = default;
};

class GreaterThanConstraint : public Constraint {
	int target;

  public:
	GreaterThanConstraint(std::vector<Position> tiles, int target);
	int getTarget() const;
	virtual bool evaluate(std::span<int> values) const override;
	virtual bool evaluate(std::span<Variable> values) const override;
	virtual bool evaluate(std::span<Tile> values) const override;
	~GreaterThanConstraint() = default;
};

class ExactSumConstraint : public Constraint {
	int target;

  public:
	ExactSumConstraint(std::vector<Position> tiles, int target);
	int getTarget() const;
	virtual bool evaluate(std::span<int> values) const override;
	virtual bool evaluate(std::span<Variable> values) const override;
	virtual bool evaluate(std::span<Tile> values) const override;
	~ExactSumConstraint() = default;
};

template <int Width, int Height> class PipsAI;

template <int Width, int Height> class Pips {
	Grid<Width, Height, Tile> grid;
	std::vector<std::shared_ptr<Domino>> dominos;
	std::vector<std::shared_ptr<Constraint>> constraints;

  public:
	Pips(std::vector<Position> disabledTiles,
		 std::vector<std::shared_ptr<Domino>> dominos,
		 std::vector<std::shared_ptr<Constraint>> constraints);
	Pips(Pips<Width, Height> &&other);

	Pips(const Pips<Width, Height> &other);
	Pips<Width, Height> &operator=(const Pips<Width, Height> &other);

	void placeDomino(std::shared_ptr<Domino> domino, Position position,
					 std::pair<bool, bool> orientation);

	std::vector<Tile> getTiles(const std::vector<Position> &positions) const;

	bool isSolved() const;

	~Pips() = default;

	friend PipsAI<Width, Height>;
	friend SolverState<Width, Height>;
};

// Definitions

// Grid Class ------------------------------------------------------------------

template <int Width, int Height, typename K>
Grid<Width, Height, K>::Grid() : grid{{K()}} {}

template <int Width, int Height, typename K>
Grid<Width, Height, K>::Grid(const std::vector<Position> &disabledTiles)
	: grid{{K()}} {
	for (const auto &pos : disabledTiles) {
		if (pos.row >= Width || pos.col >= Height)
			throw std::runtime_error("Disabled tiles out of bounds");
		grid[pos.row][pos.col] = K(false);
	}
}

template <int Width, int Height, typename K>
K &Grid<Width, Height, K>::operator[](const Position &pos) {
	if (pos.row >= Height || pos.col >= Width) {
		throw std::runtime_error("Indexed position out of bounds");
	}
	return grid[pos.row][pos.col];
}

template <int Width, int Height, typename K>
const K &Grid<Width, Height, K>::operator[](const Position &pos) const {
	if (pos.row >= Height || pos.col >= Width) {
		throw std::runtime_error("Indexed position out of bounds");
	}
	return grid[pos.row][pos.col];
}

template <int Width, int Height, typename K>
Position Grid<Width, Height, K>::getOther(
	const Position &position, const std::pair<bool, bool> &orientation) const {
	if (orientation.first) {
		if (orientation.second)
			return {position.row - 1, position.col};
		else
			return {position.row + 1, position.col};
	} else {
		if (orientation.second)
			return {position.row, position.col - 1};
		else
			return {position.row, position.col + 1};
	}
}

template <int Width, int Height, typename K>
std::array<std::array<K, Width>, Height>
Grid<Width, Height, K>::getGrid() const {
	return grid;
}

// End Grid Class --------------------------------------------------------------

// PipsState Class -------------------------------------------------------------

template <int Width, int Height>
Pips<Width, Height>::Pips(std::vector<Position> disabledTiles,
						  std::vector<std::shared_ptr<Domino>> dominos,
						  std::vector<std::shared_ptr<Constraint>> constraints)
	: constraints(std::move(constraints)), dominos(std::move(dominos)),
	  grid(disabledTiles) {}

template <int Width, int Height>
Pips<Width, Height>::Pips(Pips<Width, Height> &&other)
	: constraints(std::move(other.constraints)),
	  dominos(std::move(other.dominos)), grid(std::move(other.grid)) {}

template <int Width, int Height>
Pips<Width, Height>::Pips(const Pips &other)
	: dominos(other.dominos), constraints(other.constraints), grid(other.grid) {
}

template <int Width, int Height>
Pips<Width, Height> &Pips<Width, Height>::operator=(const Pips &other) {
	constraints = other.constraints;
	dominos = other.dominos;
	grid = other.grid;
	return *this;
}

template <int Width, int Height>
void Pips<Width, Height>::placeDomino(std::shared_ptr<Domino> domino,
									  Position position,
									  std::pair<bool, bool> orientation) {
	auto [val1, val2] = domino->getValues();
	auto pos2 = grid.getSecondTilePos(position, orientation);
	grid[position].setState(val1, domino, orientation);
	grid[pos2].setState(val2, domino, {orientation.first, !orientation.second});
}

template <int Width, int Height>
std::vector<Tile>
Pips<Width, Height>::getTiles(const std::vector<Position> &positions) const {
	std::vector<Tile> values{static_cast<int>(positions.size())};
	std::transform(positions.begin(), positions.end(), values.begin(),
				   [&](const Position &pos) { return grid[pos]; });
	return values;
}

template <int Width, int Height> bool Pips<Width, Height>::isSolved() const {
	for (const auto &row : grid) {
		for (const Tile &tile : row) {
			if (!tile.getDomino() && tile.inPlay)
				return false;
		}
	}

	for (const auto &constraint : constraints) {
		std::vector<Position> positions = constraint->getPositions();
		std::vector<int> values{static_cast<int>(positions.size())};
		std::transform(positions.begin(), positions.end(), values.begin(),
					   [&](Position &position) {
						   Tile tile = grid[position];
						   return tile.getValue();
					   });
		if (!constraint->evaluate(values))
			return false;
	}

	return true;
}

#endif
