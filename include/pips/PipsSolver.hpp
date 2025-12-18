#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <vector>

// Declarations

class Variable {
	std::set<Tile> domain;
	std::vector<std::set<Tile>> reductions;
	Position position;

  public:
	const bool inPlay;
	Variable(bool active = true);
	Variable(Tile tile);
	Variable &operator=(const Variable &other);

	void undo();
	void insertInDomain(std::set<Tile> &values);
	void pruneDomain(const std::set<Tile> &values);
	void pushReduction(const std::set<Tile> &values);
	void assign(const Tile &value);
	std::set<Tile> getDomain() const { return domain; };

	~Variable() = default;
};

struct Assignment {
	Position position;
	Tile value;
};

template <int Width, int Height> class SolverState {
	Grid<Width, Height, Variable> grid;

  public:
	SolverState(std::vector<Position> disabledTiles);
	SolverState(const Pips<Width, Height> &startState);

	void initializeDomains(const std::vector<std::shared_ptr<Domino>> &dominos);

	SolverState(const SolverState<Width, Height> &other);
	SolverState<Width, Height>
	operator=(const SolverState<Width, Height> &other);

	bool isSolvable(const Constraint &constraint);

	std::unordered_map<Position, std::set<Tile>> pruneVariableDomain(
		Position position,
		const std::set<Tile>
			&removed); // For each value in removed, it removes the value from
					   // the domain, and also the complement value from the
					   // complement variable's domain.

	void undoReduction(
		const std::unordered_map<Position, std::set<Tile>> &reductionMap);

	std::unordered_map<Position, std::set<Tile>>
	assignVariable(Position position, Tile value);

	bool isSolved() const;
	Position MRV() const;

	~SolverState() = default;

	friend PipsAI<Width, Height>;
};

template <int Width, int Height> class PipsAI {
	SolverState<Width, Height> state = nullptr;
	std::vector<std::shared_ptr<Constraint>> constraints;
	std::vector<std::shared_ptr<Domino>> dominos;
	std::stack<std::pair<SolverState<Width, Height>, Assignment>>
		frontier; // State-Action pairs

	bool solved() const;

  public:
	PipsAI(std::vector<Position> disabledTiles,
		   std::vector<std::shared_ptr<Domino>> dominos,
		   std::vector<std::shared_ptr<Constraint>> constraints);
	PipsAI(const Pips<Width, Height> &startState);
	std::optional<SolverState<Width, Height>> solve();

	bool revise(Position position, const Constraint &constraint);
	bool GAC();

	// TODO:

	void pushState(
		Assignment); // Push a <state, assignment> i.e. state-action pair
	void popState(); // Pops the top pair, assigns the variable in state,
					 // reduces direct domain conflicts, and sets state variable
};

// Definitions

template <int Width, int Height>
void SolverState<Width, Height>::initializeDomains(
	const std::vector<std::shared_ptr<Domino>> &dominos) {
	std::pair<int, int> values;
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			if (i - 1 >= 0) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						Tile(values.first, domino, Orientation::Left));
					grid[{i, j}].insertInDomain(
						Tile(values.second, domino, Orientation::Left));
				}
			}
			if (i + 1 < Height) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						Tile(values.first, domino, Orientation::Right));
					grid[{i, j}].insertInDomain(
						Tile(values.second, domino, Orientation::Right));
				}
			}
			if (j - 1 >= 0) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						Tile(values.first, domino, Orientation::Up));
					grid[{i, j}].insertInDomain(
						Tile(values.second, domino, Orientation::Up));
				}
			}
			if (j + 1 < Width) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						Tile(values.first, domino, Orientation::Down));
					grid[{i, j}].insertInDomain(
						Tile(values.second, domino, Orientation::Down));
				}
			}
		}
	}
}

template <int Width, int Height>
SolverState<Width, Height>::SolverState(std::vector<Position> disabledTiles)
	: grid{disabledTiles} {}

template <int Width, int Height>
SolverState<Width, Height>::SolverState(const Pips<Width, Height> &startState)
	: grid() {
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			grid[{i, j}] = Variable(startState[{i, j}]);
		}
	}
}

template <int Width, int Height>
SolverState<Width, Height>::SolverState(const SolverState<Width, Height> &other)
	: grid(other.grid) {}

template <int Width, int Height>
bool SolverState<Width, Height>::isSolved() const {
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			if (grid[{i, j}].inPlay && grid[{i, j}].getDomain().size() > 1)
				return false;
		}
	}
	return true;
}

template <int Width, int Height>
std::unordered_map<Position, std::set<Tile>>
SolverState<Width, Height>::assignVariable(Position position, Tile value) {
	std::set<Tile> domain = grid[position].getDomain();
	if (domain.find(value) == domain.end())
		throw std::runtime_error("Cannot assign to a value not in domain");
	domain.erase(value); // domain now holds elements to be removed
	return pruneVariableDomain(position, domain);
}

template <int Width, int Height>
std::unordered_map<Position, std::set<Tile>>
SolverState<Width, Height>::pruneVariableDomain(Position position,
												const std::set<Tile> &removed) {
	std::set<Tile> domain = grid[position].getDomain();
	std::unordered_map<Position, std::set<Tile>> reductionMap{};
	const std::set<Tile> removedComplements;
	Tile complementRemoval;
	Position complementPosition;
	for (const auto value : removed) {
		if (domain.find(value) == domain.end())
			continue;
		complementRemoval = value.getComplement();
		complementPosition = grid.getOther(position, value.getOrientation());
		grid[complementPosition].pruneDomain(complementRemoval);
		grid[position].pruneDomain(value);
		reductionMap[position].insert(value);
		reductionMap[complementPosition].insert(complementRemoval);
	}
	return reductionMap;
}

template <int Width, int Height>
void SolverState<Width, Height>::undoReduction(
	const std::unordered_map<Position, std::set<Tile>> &reductionMap) {
	for (const auto &pair : reductionMap) {
		grid[pair.first].insertInDomain(pair.second);
	}
}

template <int Width, int Height>
SolverState<Width, Height>
SolverState<Width, Height>::operator=(const SolverState<Width, Height> &other) {
	grid = other.grid;
}

template <int Width, int Height>
Position SolverState<Width, Height>::MRV() const {
	// Returns the variable (and its domain) with the most
	// constrained domain.
	// --> Grid square with the minimum possible states after
	// pruning.
	Position minVar{-1, -1};
	int minSize = INFINITY;
	int currSize;
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			if (!grid[{i, j}].inPlay)
				continue;
			currSize = grid[{i, j}].getDomain().size();
			if (currSize != 1 && currSize < minSize) {
				minSize = currSize;
				minVar = Position{i, j};
			}
		}
	}
	return minVar;
}

template <int Width, int Height>
PipsAI<Width, Height>::PipsAI(
	std::vector<Position> disabledTiles,
	std::vector<std::shared_ptr<Domino>> dominos,
	std::vector<std::shared_ptr<Constraint>> constraints)
	: state(std::make_shared<SolverState<Width, Height>>(disabledTiles, dominos,
														 constraints)),
	  frontier{} {}

template <int Width, int Height>
PipsAI<Width, Height>::PipsAI(const Pips<Width, Height> &startState)
	: state(startState), frontier{} {}

template <int Width, int Height>
bool PipsAI<Width, Height>::revise(Position position,
								   const Constraint &constraint) {
	std::set<Tile> domain = state.grid[position].getDomain();
	std::unordered_map<Position, std::set<Tile>> reductionMap;
	bool revised = false;
	for (const auto &value : domain) {

		reductionMap = state.assignVariable(position, value);
		bool isSolvable = state.isSolvable(constraint);
		state.undoReduction(reductionMap);

		if (!isSolvable) {
			state.pruneVariableDomain(position, {value});
			revised = true;
		}
	}
	return revised;
}

template <int Width, int Height>
bool SolverState<Width, Height>::isSolvable(const Constraint &constraint) {
	Position choice{-1, -1};
	auto positions = constraint.getPositions();
	std::vector<Variable> variables;
	variables.reserve(positions.size());

	for (auto pos : positions) {
		if (!grid[pos].inPlay)
			throw std::runtime_error(
				"Constraint can't constraint out of play tiles");
		variables.push_back(grid[pos]);
		if (grid[pos].getDomain().size() > 1) {
			choice = pos;
		}
	}

	if (choice.row == -1 && choice.col == -1)
		return constraint.evaluate(variables);

	std::set<Tile> domain = grid[choice].getDomain();
	std::unordered_map<Position, std::set<Tile>> reductionMap;
	for (const auto &value : domain) {
		reductionMap = assignVariable(choice, value);

		// TODO: if (!isImpossible && isSolvable) return true;  (Early out for
		// cheaper computation)

		if (isSolvable(constraint))
			return true;
		undoReduction(reductionMap);
	}

	return false;
}

template <int Width, int Height> bool PipsAI<Width, Height>::GAC() {
	std::queue<std::pair<std::shared_ptr<Constraint>, Position>> queue{};
	std::pair<std::shared_ptr<Constraint>, Position> cPair;
	// Add every <C, X> pair where X in scope(C)
	for (const auto &constraint : constraints) {
		for (const auto &position : constraint->getPositions()) {
			queue.emplace(constraint, position);
		}
	}

	while (!queue.empty()) {
		cPair = queue.front();
		queue.pop();

		if (revise(cPair.second, cPair.first)) {

			if (state.grid[cPair.second].size() == 0) {
				return false; // unsolvable
			}

			// Add every <C`, Z> pair where C` != C && X in scope(C`) && X != Z
			for (const auto &constraint : constraints) {
				if (constraint == cPair.first)
					continue;
				auto positions = constraint->getPositions();
				if (std::find(positions.begin(), positions.end(),
							  cPair.second) != positions.end()) {
					for (const auto &position : positions) {
						if (position == cPair.second)
							continue;
						queue.emplace(constraint, position);
					}
				}
			}
		}
	}
	return true;
}

template <int Width, int Height> bool PipsAI<Width, Height>::solved() const {

	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			if (!state.grid[{i, j}].inPlay)
				continue;
			if (state.grid[{i, j}].getDomain().size() != 1)
				return false;
		}
	}

	for (const auto &constraint : constraints) {
		std::vector<Position> positions = constraint->getPositions();
		std::vector<Variable> variables;
		variables.reserve(positions.size());
		for (const auto &pos : positions) {
			variables.push_back(state.grid[pos]);
		}
		if (!constraint->evaluate(variables))
			return false;
	}

	return true;
}

template <int Width, int Height>
std::optional<SolverState<Width, Height>> PipsAI<Width, Height>::solve() {

	// Frontier has <S, A> state-action pairs. Start state has no prev. action
	frontier.emplace(state, {{-1, -1}, {}});

	while (!frontier.empty()) {
		popState();	 // Pop <S, A> and set new state S` <- T(S, A)
		if (GAC()) { // Run General Arc Consistency returns false if unsolvable

			if (solved())
				return {
					state}; // TODO: solved() and Pips(SolverState) consturcter

			Position pos = state.MRV();
			// If MRV returns (-1, -1) it means |X| <= 1 for all X. But we have
			// !solved() -> something went very wrong because 0-size domains
			// should never exists.
			assert(pos.row != -1 && pos.col != -1);

			for (const auto &value : state.grid[pos].getDomain()) {
				pushState({pos, value});
			}
		}
	}

	return std::nullopt;
}

#endif
