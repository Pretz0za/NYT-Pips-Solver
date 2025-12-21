#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstddef>
#include <memory>
#include <optional>
#include <queue>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Declarations

class Variable {
	std::unordered_set<Tile> domain;
	bool assigned = false;
	bool active;

  public:
	Variable(bool inPlay = true);
	Variable(Tile tile);
	Variable &operator=(const Variable &other);
	bool inPlay() const { return active; };

	void setAssigned();
	bool isAssigned() const;
	void insertInDomain(std::unordered_set<Tile> values);
	void pruneDomain(const Tile &values);
	void pushReduction(const std::unordered_set<Tile> &values);
	void clearDomain() { domain.clear(); }
	std::unordered_set<Tile> clearDomino(std::shared_ptr<Domino> domino);
	std::unordered_set<Tile> getDomain() const { return domain; };

	void printTile(int x) const;

	~Variable() = default;
};

struct Assignment {
	Position position;
	Tile value;
};

template <int Width, int Height> class SolverState {
	Grid<Width, Height, Variable> grid;

	std::stack<std::unordered_map<Position, std::unordered_set<Tile>>>
		reductions;
	std::unordered_map<Position, std::unordered_set<Tile>> reductionMap;

	void clearDominoFromDomains(std::shared_ptr<Domino> domino);
	void clearDomain(const Position &position);

	void pushReductionMap();

  public:
	SolverState(std::vector<Position> disabledTiles);
	SolverState(const Pips<Width, Height> &startState);
	SolverState(const SolverState<Width, Height> &other);
	SolverState<Width, Height>
	operator=(const SolverState<Width, Height> &other);

	void initializeDomains(const std::vector<std::shared_ptr<Domino>> &dominos);

	void undoReduction();
	void printSolution(const std::string &str) const;

	bool isSolvable(const Constraint &constraint);
	bool isImpossible(const Constraint &constraint) const;

	void pruneVariableDomain(
		Position position,
		const Tile &removed); // For each value in removed, it removes the value
							  // from the domain, and also the complement value
							  // from the complement variable's domain.

	std::unordered_map<Position, std::unordered_set<Tile>>
	assignVariable(Position position, Tile value);

	// Returns true if the last reduction cleared a variables domain
	bool causedEmptyDomain() const;
	bool isSolved() const;
	Position MRV() const;

	~SolverState() = default;

	friend PipsAI<Width, Height>;
};

template <int Width, int Height> class PipsAI {
	SolverState<Width, Height> state;
	std::vector<std::shared_ptr<Constraint>> constraints;
	std::vector<std::shared_ptr<Domino>> dominos;
	std::stack<std::pair<SolverState<Width, Height>, Assignment>>
		frontier; // State-Action pairs

	bool solved() const;

	bool revise(Position position, const Constraint &constraint);
	bool GAC();

	// TODO:

	void pushState(
		Assignment action); // Push a <state, assignment> i.e. state-action pair
	bool popState();		// Pops the top pair, assigns the variable in state,
					 // reduces direct domain conflicts, and sets state variable

  public:
	PipsAI(std::vector<Position> disabledTiles,
		   std::vector<std::shared_ptr<Domino>> dominos,
		   std::vector<std::shared_ptr<Constraint>> constraints);
	PipsAI(const Pips<Width, Height> &startState);
	std::optional<SolverState<Width, Height>> solve();
};

// Definitions

template <int Width, int Height>
void SolverState<Width, Height>::initializeDomains(
	const std::vector<std::shared_ptr<Domino>> &dominos) {
	std::pair<int, int> values;
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {

			if (!grid[{i, j}].inPlay())
				continue;

			if (j - 1 >= 0 && grid[{i, j - 1}].inPlay()) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						{Tile(true, domino, Orientation::Left)});
					grid[{i, j}].insertInDomain(
						{Tile(false, domino, Orientation::Left)});
				}
			}
			if (j + 1 < Width && grid[{i, j + 1}].inPlay()) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						{Tile(true, domino, Orientation::Right)});
					grid[{i, j}].insertInDomain(
						{Tile(false, domino, Orientation::Right)});
				}
			}
			if (i - 1 >= 0 && grid[{i - 1, j}].inPlay()) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						{Tile(true, domino, Orientation::Up)});
					grid[{i, j}].insertInDomain(
						{Tile(false, domino, Orientation::Up)});
				}
			}
			if (i + 1 < Height && grid[{i + 1, j}].inPlay()) {
				for (const auto &domino : dominos) {
					values = domino->getValues();
					grid[{i, j}].insertInDomain(
						{Tile(true, domino, Orientation::Down)});
					grid[{i, j}].insertInDomain(
						{Tile(false, domino, Orientation::Down)});
				}
			}
		}
	}
}

template <int Width, int Height>
void SolverState<Width, Height>::pushReductionMap() {
	reductions.push(reductionMap);
	reductionMap.clear();
}

template <int Width, int Height>
void SolverState<Width, Height>::printSolution(const std::string &str) const {
	int x = 1;
	int y = 2;
	std::cout << "\x1b[?7l"; // disable auto wrap
	std::cout << "\x1b[2J\x1b[H";
	for (int i = 0; i < Height; i++, y += 4) {
		x = 1;
		for (int j = 0; j < Width; j++, x += 8) {
			if (!grid[{i, j}].inPlay())
				continue;
			Tile value = *grid[{i, j}].getDomain().begin();
			value.printTile(x, y - 1);
		}
	}
	std::cout << '\n' << str << '\n';
}

template <int Width, int Height>
void SolverState<Width, Height>::clearDomain(const Position &position) {
	std::unordered_set<Tile> domain = grid[position].getDomain();

	for (const auto &value : domain) {
		// Remove Complements
		Position complementPosition =
			grid.getOther(position, value.getOrientation());
		grid[complementPosition].pruneDomain({value.getComplement()});
		reductionMap[complementPosition].insert(value.getComplement());
	}

	reductionMap[position].merge(domain);
	grid[position].clearDomain(); // TODO: make this;
}

template <int Width, int Height>
void SolverState<Width, Height>::clearDominoFromDomains(
	std::shared_ptr<Domino> domino) {
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			reductionMap[{i, j}].merge(grid[{i, j}].clearDomino(domino));
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
			if (grid[{i, j}].inPlay() && grid[{i, j}].getDomain().size() > 1)
				return false;
		}
	}
	return true;
}

template <int Width, int Height>
std::unordered_map<Position, std::unordered_set<Tile>>
SolverState<Width, Height>::assignVariable(Position position, Tile value) {
	if (!grid.inBounds(position))
		throw std::runtime_error("Can't assign to out of bounds Variable");

	Tile complement = value.getComplement();
	Position complementPosition =
		grid.getOther(position, value.getOrientation());

	clearDominoFromDomains(value.getDomino()); // TODO: Make this;
	clearDomain(position);					   // TODO: Make this;
	clearDomain(complementPosition);

	grid[position].insertInDomain({value});
	grid[complementPosition].insertInDomain({complement});

	pushReductionMap(); // TODO: Make this;
	return reductions.top();
}

template <int Width, int Height>
void SolverState<Width, Height>::pruneVariableDomain(Position position,
													 const Tile &removed) {
	grid[position].pruneDomain(removed);
	grid[grid.getOther(position, removed.getOrientation())].pruneDomain(
		removed.getComplement());
}

template <int Width, int Height>
void SolverState<Width, Height>::undoReduction() {
	for (const auto &pair : reductions.top()) {
		grid[pair.first].insertInDomain(pair.second);
	}
	reductions.pop();
}

template <int Width, int Height>
SolverState<Width, Height>
SolverState<Width, Height>::operator=(const SolverState<Width, Height> &other) {
	grid = other.grid;
	return *this;
}

template <int Width, int Height>
Position SolverState<Width, Height>::MRV() const {
	// Returns the unassigned variable with the most constrained domain.
	// --> Grid square with the minimum possible states after pruning.
	Position minVar{-1, -1};
	int minSize = INT_MAX;
	int currSize;
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			if (!grid[{i, j}].inPlay() || grid[{i, j}].isAssigned())
				continue;
			currSize = grid[{i, j}].getDomain().size();
			if (currSize < minSize) {
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
	: dominos{dominos}, constraints{constraints}, state{disabledTiles},
	  frontier{} {}

template <int Width, int Height>
PipsAI<Width, Height>::PipsAI(const Pips<Width, Height> &startState)
	: state(startState), frontier{} {}

template <int Width, int Height>
bool SolverState<Width, Height>::causedEmptyDomain() const {
	for (const auto &p : reductions.top()) {
		if (!p.second.empty() && grid[p.first].getDomain().size() == 0)
			return true;
	}
	return false;
}

template <int Width, int Height>
bool PipsAI<Width, Height>::revise(Position position,
								   const Constraint &constraint) {
	std::unordered_set<Tile> domain = state.grid[position].getDomain();
	bool revised = false;
	if (domain.size() == 1)
		return revised;

	bool isSolvable, isBroken, isIncompatible;
	for (const auto &value : domain) {

		state.assignVariable(position, value);

		isBroken = state.isImpossible(constraint);
		if (!isBroken) {
			isIncompatible = state.causedEmptyDomain();
			if (!isIncompatible)
				isSolvable = state.isSolvable(constraint);
		}

		state.undoReduction();

		if (isBroken || isIncompatible || !isSolvable) {
			state.pruneVariableDomain(position, value);
			revised = true;
		}
	}

	return revised;
}

template <int Width, int Height>
bool SolverState<Width, Height>::isSolvable(const Constraint &constraint) {
	Position choice{-1, -1};
	auto positions = constraint.getPositions();

	for (const auto &pos : positions) {
		if (grid[pos].getDomain().size() == 0)
			return false;
		if (grid[pos].getDomain().size() > 1) {
			choice = pos;
		}
	}

	if (!grid.inBounds(choice)) {
		std::vector<int> values;
		values.reserve(positions.size());

		for (const auto &pos : positions) {
			values.push_back(grid[pos].getDomain().begin()->getValue());
		}

		return constraint.evaluate(values);
	}

	std::unordered_set<Tile> domain = grid[choice].getDomain();
	bool broken, solved, incompatible;
	for (const auto &value : domain) {
		// TODO: if (!isImpossible && isSolvable) return true;  (Early out for
		// cheaper computation)

		assignVariable(choice, value);

		broken = this->isImpossible(constraint);
		if (!broken) {
			incompatible = this->causedEmptyDomain();
			if (!incompatible)
				solved = this->isSolvable(constraint);
		}

		undoReduction();

		if (!broken && !incompatible && solved)
			return true;
	}

	return false;
}

template <int Width, int Height>
bool SolverState<Width, Height>::isImpossible(
	const Constraint &constraint) const {
	std::vector<Position> positions = constraint.getPositions();
	std::vector<int> assignedValues;
	assignedValues.reserve(positions.size());
	for (const auto &pos : positions) {
		if (grid[pos].getDomain().size() == 1) {
			assignedValues.push_back(grid[pos].getDomain().begin()->getValue());
		}
	}
	return constraint.isBroken(assignedValues);
}

template <int Width, int Height> bool PipsAI<Width, Height>::GAC() {
	std::queue<std::pair<std::shared_ptr<Constraint>, Position>>
		queue{}; // TODO: Change to priority queue with |D(X)|
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

		if (revise(cPair.second, *cPair.first)) {

			if (state.grid[cPair.second].getDomain().size() == 0) {
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
			if (!state.grid[{i, j}].inPlay())
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
void PipsAI<Width, Height>::pushState(Assignment action) {
	frontier.emplace(state, action);
}

template <int Width, int Height> bool PipsAI<Width, Height>::popState() {
	std::pair<SolverState<Width, Height>, Assignment> pair = frontier.top();
	frontier.pop();
	state = pair.first;
	bool valid = true;
	if (pair.second.position.row != -1 && pair.second.position.col != -1) {
		state.assignVariable(pair.second.position, pair.second.value);
		state.grid[pair.second.position].setAssigned();
		state
			.grid[state.grid.getOther(pair.second.position,
									  pair.second.value.getOrientation())]
			.setAssigned();
		valid = !state.causedEmptyDomain();
	}

	if (!valid) {
		if (!frontier.empty())
			return popState();
	}
	return valid;
}

template <int Width, int Height>
std::optional<SolverState<Width, Height>> PipsAI<Width, Height>::solve() {

	auto t = std::chrono::high_resolution_clock::now();
	std::cout << "initializing variable domains...\n";
	state.initializeDomains(dominos);
	std::cout << "initialized. took: "
			  << std::chrono::duration_cast<std::chrono::milliseconds>(
					 std::chrono::high_resolution_clock::now() - t)
			  << '\n';

	for (const auto &constraint : constraints) {
		std::vector<int> impossible = constraint->impossibleValues(dominos);
		for (const auto &position : constraint->getPositions()) {
			for (const Tile &value : state.grid[position].getDomain()) {
				if (std::find(impossible.begin(), impossible.end(),
							  value.getValue()) != impossible.end()) {
					state.pruneVariableDomain(position, value);
				}
			}
		}
	}

	Position pos = state.MRV();
	if (pos.row != -1 && pos.col != -1) {
		for (const auto &value : state.grid[pos].getDomain()) {
			pushState({pos, value});
		}
	}

	// size_t maxSize = 0;
	// std::shared_ptr<Constraint> maxSizeConstraint = nullptr;
	// for (const auto &constraint : constraints) {
	// 	if (constraint->getPositions().size() >= maxSize) {
	// 		maxSize = constraint->getPositions().size();
	// 		maxSizeConstraint = constraint;
	// 	}
	// }
	// int minSize = 100000;
	// Position minSizePosition{-1, -1};
	// for (const auto &pos : maxSizeConstraint->getPositions()) {
	// 	if (state.grid[pos].getDomain().size() < minSize) {
	// 		minSize = state.grid[pos].getDomain().size();
	// 		minSizePosition = pos;
	// 	}
	// }
	//
	// for (const Tile &value : state.grid[minSizePosition].getDomain()) {
	// 	frontier.emplace(state, Assignment{minSizePosition, value});
	// }
	//
	// Frontier has <S, A> state-action pairs. Start state has no prev. action
	// frontier.emplace(state, Assignment{Position{-1, -1}, Tile{}});

	std::vector<std::chrono::milliseconds> times{};

	std::cout << "beginning pruning...\n";
	while (!frontier.empty()) {
		// Pop <S, A> and set new state S` <- T(S, A). Return false if there is
		// no valid S' in frontier.
		auto t = std::chrono::high_resolution_clock::now();
		if (popState() &&
			GAC()) { // Run General Arc Consistency returns false if unsolvable

			if (solved()) {
				int i = 1;
				for (const auto &time : times) {
					std::cout << "reduction/assignment #" << i
							  << " took: " << time << '\n';
					i++;
				}
				return {state};
			} // TODO: solved() and Pips(SolverState) consturcter

			Position pos = state.MRV();
			// If MRV returns (-1, -1) it means |X| <= 1 for all X. But we have
			// !solved() -> something went very wrong because 0-size domains
			// should never exists.
			if (pos.row != -1 && pos.col != -1) {
				for (const auto &value : state.grid[pos].getDomain()) {
					pushState({pos, value});
				}
			}
		}
		times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - t));
	}

	return std::nullopt;
}

#endif
