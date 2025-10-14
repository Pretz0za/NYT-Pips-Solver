#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include <vector>
#include <tuple>
#include <initializer_list>

struct Position {
	int x;
	int y;
};

class Domino {
	std::tuple<int> value;
	Position position;
	// 4 possible orientations give us:
	std::tuple<bool> orientation;
public:
	
	Domino(std::tuple<int> values, Position position, std::tuple<bool> orientaion);

	// Returns the value of the given domino face, i.e. value[index]
	int getFace(int index) const;
	~Domino() = default;
};

class Tile {
	int value;
public:

	Tile(int value = -1);

	// gets/sets the value of the tile and errors if the tile is out of play
	int getValue() const;
	int setValue(int newValue) const;

	~Tile() = default;
};

class Constraint {
	std::vector<Position> tiles;

public:
	std::vector<Position> getTiles();
	virtual bool evaluate(std::initializer_list<int> values) const;
	virtual ~Constraint() = default;
};

class PipsState {
	std::vector<std::vector<Tile>> grid;
	std::vector<Domino*> dominos;
	const std::vector<Constraint> constraints;

public:
	PipsState();

	bool isSolved() const;
	std::vector<PipsState> makeNeighbors() const;
	int objective() const;

	~PipsState() = default;
};

#endif
