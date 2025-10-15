#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include <initializer_list>
#include <memory>
#include <vector>

struct Position {
  int x;
  int y;
};

class Domino {
  std::pair<int, int> value;
  Position position;
  // 4 possible orientations give us:
  std::pair<bool, bool> orientation;

public:
  Domino(std::pair<int, int> value);

  void setPosition(const Position &newPosition);
  void setOrientation(const std::pair<bool, bool> &newOrientation);

  void rotate();
  void swap(Domino &other);

  // Returns the values of the domino
  std::pair<int, int> getValues() const;

  // Returns the positions of both faces in a pair based on orientation
  std::pair<Position, Position> getPosition() const;

  ~Domino() = default;
};

class Tile {
  int value;

public:
  Tile(int value = -1);

  // gets/sets the value of the tile and errors if the tile is out of play
  int getValue() const;
  void setValue(int newValue);

  ~Tile() = default;
};

class Constraint {
protected:
  std::vector<Position> tiles;

public:
  Constraint(std::vector<Position> tiles);

  std::vector<Position> getTiles() const;
  virtual bool evaluate(std::initializer_list<int> values) const;

  virtual ~Constraint() = default;
};

class EqualConstraint : public Constraint {
public:
  EqualConstraint(std::vector<Position> tiles);
  bool evaluate(std::initializer_list<int> values) const override;
};

class UniqueConstraint : public Constraint {
public:
  UniqueConstraint(std::vector<Position> tiles);
  bool evaluate(std::initializer_list<int> values) const override;
};

class LessThanConstraint : public Constraint {
  int limit;

public:
  LessThanConstraint(std::vector<Position> tiles, int limit);
  bool evaluate(std::initializer_list<int> values) const override;
};

class GreaterThanConstraint : public Constraint {
  int limit;

public:
  GreaterThanConstraint(std::vector<Position> tiles, int limit);
  bool evaluate(std::initializer_list<int> values) const override;
};

class PipsState {
  std::vector<std::vector<Tile>> grid;
  std::vector<Domino> dominos;
  std::shared_ptr<const std::vector<Constraint>> constraints;

public:
  PipsState(int width, int height, std::vector<Position> disabledTiles,
            std::vector<Domino> dominos,
            std::shared_ptr<std::vector<Constraint>> constraints);

  void rotateDomino(int index);
  void swapDominos(int first, int second);
  void placeDomino(const Domino &domino);

  bool isSolved() const;
  std::vector<PipsState> makeNeighbors() const;
  int objective() const;

  ~PipsState() = default;
};

#endif
