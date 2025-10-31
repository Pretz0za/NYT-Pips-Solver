#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stack>
#include <vector>

// Declarations

template <int Width, int Height> class SolverState {
	PipsState<Width, Height> gameState;
	std::map<Position, std::set<Tile>> domains;

  public:
	SolverState(PipsState<Width, Height> gameState);
	SolverState(
		std::vector<Position> disabledTiles,
		std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos,
		std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>>
			constraints);

	SolverState(const SolverState<Width, Height> &other);
	SolverState<Width, Height>
	operator=(const SolverState<Width, Height> &other);

	std::pair<Position, std::set<Tile>> MRV() const;

	~SolverState() = default;
};

template <int Width, int Height> class PipsSolver {
	PipsState<Width, Height> initialState;
	std::unique_ptr<SolverState<Width, Height>> currentState = nullptr;
	std::stack<std::unique_ptr<SolverState<Width, Height>>> stack;
	Grid<Width, Height> grid;
	std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos;

  public:
	std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints;
	PipsSolver(
		std::vector<Position> disabledTiles,
		std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos,
		std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>>
			constraints);
	PipsSolver(PipsState<Width, Height> startState);
	std::optional<PipsState<Width, Height>> solve();
};

// Definitions

template <int Width, int Height>
SolverState<Width, Height>::SolverState(PipsState<Width, Height> gameState)
	: gameState(gameState), domains{} {}

template <int Width, int Height>
SolverState<Width, Height>::SolverState(
	std::vector<Position> disabledTiles,
	std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos,
	std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints)
	: gameState{disabledTiles, dominos, constraints}, domains{} {}

template <int Width, int Height>
SolverState<Width, Height>::SolverState(const SolverState<Width, Height> &other)
	: gameState(other.gameState), domains{other.domains} {}

template <int Width, int Height>
SolverState<Width, Height>
SolverState<Width, Height>::operator=(const SolverState<Width, Height> &other) {
	gameState = other.gameState;
	domains = other.domains;
}

template <int Width, int Height>
std::pair<Position, std::set<Tile>> SolverState<Width, Height>::MRV() const {
	// Returns the variable (and its domain) with the most constrained domain.
	// --> Grid square with the minimum possible states after pruning.
	Position maxP{};
	int maxSize = -1;
	for (const auto &[pos, domain] : domains) {
		if (domain.size() > maxSize) {
			maxSize = domain.size();
			maxP = pos;
		}
	}
	return {maxP, domains.at(maxP)};
}

template <int Width, int Height>
PipsSolver<Width, Height>::PipsSolver(
	std::vector<Position> disabledTiles,
	std::shared_ptr<const std::vector<std::shared_ptr<Domino>>> dominos,
	std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints)
	: dominos(std::move(dominos)), constraints(std::move(constraints)),
	  grid(disabledTiles), initialState{disabledTiles, dominos, constraints} {}

template <int Width, int Height>
PipsSolver<Width, Height>::PipsSolver(PipsState<Width, Height> startState)
	: initialState(startState), dominos(startState.dominos),
	  constraints(startState.constraints), grid(startState.grid) {}

template <int Width, int Height>
std::optional<PipsState<Width, Height>> PipsSolver<Width, Height>::solve() {
	stack.push(std::make_unique<SolverState>(initialState));
	while (!stack.empty()) {
		currentState = std::move(stack.top());
		stack.pop();

		if (currentState->isSolved()) {
			return currentState->getState();
		}

		auto [pos, domain] = currentState->MRV();
		for (const auto &value : domain) {
			std::unique_ptr<SolverState<Width, Height>> next =
				currentState->setVariable(pos, value);

			// TODO: filter out bad candidates here
			// - Check arc consistency
			// - Check effects on neighboring constraints
			// -- Etc...

			stack.push(std::move(next));
		}
	}
}

#endif
