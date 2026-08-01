#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <vector>

// A solver tile is fully determined by (domino index d, side s, orientation o).
// It is encoded as TileId = d*8 + s*4 + o, where o = (vertical << 1) | second
// matches the Orientation pairs:
//   Right{false,false}=0, Left{false,true}=1, Down{true,false}=2, Up{true,true}=3
// complement(id) = id ^ 5 flips the side bit (bit 2) and orientation.second
// (bit 0), matching Tile::getComplement.
using TileId = uint16_t;

// A 256-bit fixed domain over TileIds (supports up to 32 dominos = 256 tiles).
struct DomainMask {
	std::array<uint64_t, 4> b{};

	void set(int i) { b[i >> 6] |= (uint64_t{1} << (i & 63)); }
	void clear(int i) { b[i >> 6] &= ~(uint64_t{1} << (i & 63)); }
	bool test(int i) const { return (b[i >> 6] >> (i & 63)) & 1; }

	bool empty() const { return !(b[0] | b[1] | b[2] | b[3]); }
	int size() const {
		return std::popcount(b[0]) + std::popcount(b[1]) +
			   std::popcount(b[2]) + std::popcount(b[3]);
	}

	void andWith(const DomainMask &o) {
		for (int i = 0; i < 4; i++)
			b[i] &= o.b[i];
	}
	void orWith(const DomainMask &o) {
		for (int i = 0; i < 4; i++)
			b[i] |= o.b[i];
	}
	void andNot(const DomainMask &o) {
		for (int i = 0; i < 4; i++)
			b[i] &= ~o.b[i];
	}

	bool operator==(const DomainMask &) const = default;

	// Iterate over set bit indices (TileIds), lowest first.
	template <class F> void forEach(F &&f) const {
		for (int w = 0; w < 4; w++) {
			uint64_t x = b[w];
			while (x) {
				int t = std::countr_zero(x);
				f(w * 64 + t);
				x &= x - 1;
			}
		}
	}

	// Returns first set bit index (only valid when size()==1 is intended).
	int first() const {
		for (int w = 0; w < 4; w++)
			if (b[w])
				return w * 64 + std::countr_zero(b[w]);
		return -1;
	}
};

// Declarations ----------------------------------------------------------------

template <int Width, int Height> class PipsAI;

template <int Width, int Height> class SolverState {
	static constexpr int N = Width * Height;

	std::array<DomainMask, N> domain{};
	std::array<bool, N> inPlay{};
	std::array<bool, N> assigned{};
	// Non-owning pointer to the solver's dominos, used only by printBoard to
	// decode a TileId back into a Tile for drawing. Valid while the owning
	// PipsAI is alive (true for the returned solution in main.cpp).
	const std::vector<std::shared_ptr<Domino>> *dominos = nullptr;

  public:
	SolverState() = default;
	SolverState(const std::vector<Position> &disabledTiles) {
		inPlay.fill(true);
		for (const auto &pos : disabledTiles) {
			if (pos.row < 0 || pos.row >= Height || pos.col < 0 ||
				pos.col >= Width)
				throw std::runtime_error("Disabled tiles out of bounds");
			inPlay[pos.row * Width + pos.col] = false;
		}
	}

	void printBoard(const std::string &str) const;

	~SolverState() = default;

	friend PipsAI<Width, Height>;
};

template <int Width, int Height> class PipsAI {
	static constexpr int N = Width * Height;

	SolverState<Width, Height> state;
	std::vector<std::shared_ptr<Constraint>> constraints;
	std::vector<std::shared_ptr<Domino>> dominos;

	// Precomputed tables (see TileId encoding above).
	std::array<uint8_t, 256> valueOf{};   // pip value of each TileId
	std::array<DomainMask, 32> dominoMask; // 8 TileIds owned by each domino
	int nDominos = 0;

	// Constraint scope cells (flat indices) and the GAC arc structures.
	std::vector<std::vector<int>> scopeCells;    // per constraint
	std::vector<int> arcConstraint;              // arc -> constraint index
	std::vector<int> arcScopeIdx;                // arc -> scope index
	std::vector<int> arcBase;                    // constraint -> first arc id
	std::vector<std::vector<int>> cellConstraints; // cell -> constraint indices

	// Trail for backtracking: OR each removed mask back to undo.
	struct TrailEntry {
		uint16_t cell;
		DomainMask removed;
	};
	std::vector<TrailEntry> trail;
	std::vector<uint16_t> assignTrail; // cells newly marked assigned

	// Frontier of pending branches.
	struct Branch {
		uint16_t cell;
		TileId value;
		uint32_t trailMark;
		uint32_t assignMark;
	};
	std::vector<Branch> frontier;

	// --- geometry -----------------------------------------------------------
	static int neighborOf(int cell, int o) {
		switch (o) {
		case 0:
			return cell + 1; // Right (col+1)
		case 1:
			return cell - 1; // Left (col-1)
		case 2:
			return cell + Width; // Down (row+1)
		default:
			return cell - Width; // Up (row-1)
		}
	}

	// --- trail helpers ------------------------------------------------------
	// Remove `mask` (intersected with the live domain) from cell, recording it.
	// Returns whether anything was removed.
	bool removeBits(int cell, const DomainMask &mask) {
		DomainMask actual = mask;
		actual.andWith(state.domain[cell]);
		if (actual.empty())
			return false;
		trail.push_back({static_cast<uint16_t>(cell), actual});
		state.domain[cell].andNot(actual);
		return true;
	}

	void markAssigned(int cell) {
		if (!state.assigned[cell]) {
			state.assigned[cell] = true;
			assignTrail.push_back(static_cast<uint16_t>(cell));
		}
	}

	void undoTo(uint32_t trailMark, uint32_t assignMark) {
		while (trail.size() > trailMark) {
			TrailEntry &e = trail.back();
			state.domain[e.cell].orWith(e.removed);
			trail.pop_back();
		}
		while (assignTrail.size() > assignMark) {
			state.assigned[assignTrail.back()] = false;
			assignTrail.pop_back();
		}
	}

	// --- pip value mask of a cell's domain ---------------------------------
	uint8_t valueMaskOf(int cell) const {
		uint8_t m = 0;
		state.domain[cell].forEach(
			[&](int id) { m |= static_cast<uint8_t>(1u << valueOf[id]); });
		return m;
	}

	// --- core solver steps --------------------------------------------------
	void precompute();
	void initializeDomains();
	void impossibleValuesPass();

	// Vacate `cell`: remove its whole domain and, for each tile it held, remove
	// the complementary "spanning-into-cell" tile from the pointed neighbor.
	// This is what prevents another domino from later overlapping this cell.
	void vacateCell(int cell);

	// Place domino tile `t` at `cell` (fixing cell and its partner). Records all
	// changes on the trail. Returns false if the branch is dead (some in-play
	// cell was emptied).
	bool assignVariable(int cell, TileId t);

	// GAC over all constraints. Prunes unsupported values from cell domains
	// (and complements from neighbors), recording all changes on the trail.
	// Returns false if some domain was emptied (dead branch).
	bool GAC();

	int MRV() const;
	bool solved() const;

	void pushChild(int cell, TileId value, uint32_t tMark, uint32_t aMark) {
		frontier.push_back({static_cast<uint16_t>(cell), value, tMark, aMark});
	}
	// Pop the next branch, restore to its mark, and apply its assignment.
	// Skips dead branches. Returns false if the frontier is exhausted.
	bool popState();

  public:
	PipsAI(std::vector<Position> disabledTiles,
		   std::vector<std::shared_ptr<Domino>> dominos,
		   std::vector<std::shared_ptr<Constraint>> constraints)
		: state{disabledTiles}, constraints{std::move(constraints)},
		  dominos{std::move(dominos)} {
		state.dominos = &this->dominos;
		precompute();
	}

	std::optional<SolverState<Width, Height>> solve();
};

// Definitions -----------------------------------------------------------------

template <int Width, int Height>
void SolverState<Width, Height>::printBoard(const std::string &str) const {
	int x = 1;
	int y = 2;
	std::cout << "\x1b[2J\x1b[H";
	for (int i = 0; i < Height; i++, y += 4) {
		x = 1;
		for (int j = 0; j < Width; j++, x += 8) {
			int cell = i * Width + j;
			if (!inPlay[cell] || !assigned[cell] || domain[cell].size() != 1)
				continue;
			TileId id = static_cast<TileId>(domain[cell].first());
			int d = id >> 3;
			bool side = (id >> 2) & 1;
			int o = id & 3;
			Tile value(side, (*dominos)[d],
					   {static_cast<bool>(o >> 1), static_cast<bool>(o & 1)});
			value.printTile(x, y - 1);
		}
	}
	std::cout << '\n' << str << '\n';
}

template <int Width, int Height> void PipsAI<Width, Height>::precompute() {
	nDominos = static_cast<int>(dominos.size());
	assert(nDominos <= 32 && "solver supports up to 32 dominos");

	for (int d = 0; d < nDominos; d++) {
		dominoMask[d] = DomainMask{};
		for (int s = 0; s < 2; s++) {
			for (int o = 0; o < 4; o++) {
				int id = d * 8 + s * 4 + o;
				// side bit set => domino's first value, else second.
				valueOf[id] = static_cast<uint8_t>(dominos[d]->getValue(s == 1));
				dominoMask[d].set(id);
			}
		}
	}

	// Constraint scope cells and GAC arc bookkeeping.
	cellConstraints.assign(N, {});
	scopeCells.assign(constraints.size(), {});
	arcBase.assign(constraints.size(), 0);
	int arc = 0;
	for (std::size_t c = 0; c < constraints.size(); c++) {
		arcBase[c] = arc;
		const auto &positions = constraints[c]->getPositions();
		scopeCells[c].reserve(positions.size());
		for (const auto &p : positions) {
			int cell = p.row * Width + p.col;
			scopeCells[c].push_back(cell);
			cellConstraints[cell].push_back(static_cast<int>(c));
			arcConstraint.push_back(static_cast<int>(c));
			arcScopeIdx.push_back(static_cast<int>(scopeCells[c].size() - 1));
			arc++;
		}
	}
}

template <int Width, int Height>
void PipsAI<Width, Height>::initializeDomains() {
	// For each in-play cell, add both sides of every domino for each direction
	// whose neighbor is also in play. Orientation ids: Left=1, Right=0, Up=3,
	// Down=2.
	auto inBoundsInPlay = [&](int r, int c) {
		return r >= 0 && r < Height && c >= 0 && c < Width &&
			   state.inPlay[r * Width + c];
	};
	for (int i = 0; i < Height; i++) {
		for (int j = 0; j < Width; j++) {
			int cell = i * Width + j;
			if (!state.inPlay[cell])
				continue;
			struct Dir {
				bool ok;
				int o;
			};
			Dir dirs[4] = {{inBoundsInPlay(i, j - 1), 1}, // Left
						   {inBoundsInPlay(i, j + 1), 0}, // Right
						   {inBoundsInPlay(i - 1, j), 3}, // Up
						   {inBoundsInPlay(i + 1, j), 2}}; // Down
			for (const Dir &dir : dirs) {
				if (!dir.ok)
					continue;
				for (int d = 0; d < nDominos; d++) {
					state.domain[cell].set(d * 8 + 0 * 4 + dir.o); // side 0
					state.domain[cell].set(d * 8 + 1 * 4 + dir.o); // side 1
				}
			}
		}
	}
}

template <int Width, int Height>
void PipsAI<Width, Height>::impossibleValuesPass() {
	// Root-level pruning: remove constraint-impossible pip values (and their
	// complements from neighbors) directly, without the trail.
	for (const auto &constraint : constraints) {
		std::vector<int> impossible = constraint->impossibleValues(dominos);
		if (impossible.empty())
			continue;
		uint8_t impMask = 0;
		for (int v : impossible)
			if (v >= 0 && v <= 6)
				impMask |= static_cast<uint8_t>(1u << v);

		for (const auto &p : constraint->getPositions()) {
			int cell = p.row * Width + p.col;
			DomainMask rem;
			state.domain[cell].forEach([&](int id) {
				if (impMask & (1u << valueOf[id]))
					rem.set(id);
			});
			// Remove complements from neighbors, then the values from the cell.
			rem.forEach([&](int id) {
				int nb = neighborOf(cell, id & 3);
				state.domain[nb].clear(id ^ 5);
			});
			state.domain[cell].andNot(rem);
		}
	}
}

template <int Width, int Height>
void PipsAI<Width, Height>::vacateCell(int cell) {
	DomainMask dc = state.domain[cell];
	if (dc.empty())
		return;
	// For every tile held here, its complement (the spanning-into-cell tile)
	// must be removed from the pointed neighbor. Accumulate per orientation.
	DomainMask nbRem[4];
	dc.forEach([&](int id) { nbRem[id & 3].set(id ^ 5); });
	for (int o = 0; o < 4; o++) {
		if (nbRem[o].empty())
			continue;
		int nb = neighborOf(cell, o);
		removeBits(nb, nbRem[o]);
	}
	// Clear this cell's domain entirely (recorded so it can be restored).
	removeBits(cell, dc);
}

template <int Width, int Height>
bool PipsAI<Width, Height>::assignVariable(int cell, TileId t) {
	int d = t >> 3;
	TileId comp = static_cast<TileId>(t ^ 5);
	int nb = neighborOf(cell, t & 3);

	// 1. Domino exclusivity: remove domino d from every in-play cell.
	for (int c = 0; c < N; c++) {
		if (!state.inPlay[c])
			continue;
		removeBits(c, dominoMask[d]);
	}

	// 2. Vacate cell and its partner (prevents future overlaps into them).
	vacateCell(cell);
	vacateCell(nb);

	// 3. Fix the placement.
	state.domain[cell].set(t);
	state.domain[nb].set(comp);
	markAssigned(cell);
	markAssigned(nb);

	// 4. Dead if any in-play cell was emptied.
	for (int c = 0; c < N; c++) {
		if (state.inPlay[c] && state.domain[c].empty())
			return false;
	}
	return true;
}

template <int Width, int Height> bool PipsAI<Width, Height>::GAC() {
	const int numArcs = static_cast<int>(arcConstraint.size());
	std::vector<char> inQueue(numArcs, 1);
	std::queue<int> q;
	for (int a = 0; a < numArcs; a++)
		q.push(a);

	while (!q.empty()) {
		int a = q.front();
		q.pop();
		inQueue[a] = 0;

		int ci = arcConstraint[a];
		int si = arcScopeIdx[a];
		const std::vector<int> &cells = scopeCells[ci];
		int cell = cells[si];
		const Constraint &constraint = *constraints[ci];

		// Build pip-value masks for the whole scope.
		std::vector<uint8_t> masks(cells.size());
		for (std::size_t i = 0; i < cells.size(); i++)
			masks[i] = valueMaskOf(cells[i]);

		// Find unsupported tiles in this cell's domain.
		DomainMask cellRem;
		DomainMask nbRem[4];
		state.domain[cell].forEach([&](int id) {
			int v = valueOf[id];
			if (!constraint.supportsValue(v, si, masks)) {
				cellRem.set(id);
				nbRem[id & 3].set(id ^ 5);
			}
		});

		if (cellRem.empty())
			continue; // unchanged

		removeBits(cell, cellRem);
		for (int o = 0; o < 4; o++) {
			if (!nbRem[o].empty())
				removeBits(neighborOf(cell, o), nbRem[o]);
		}

		if (state.domain[cell].empty())
			return false; // dead

		// Re-enqueue arcs (C', Z) for other constraints containing `cell`.
		for (int cj : cellConstraints[cell]) {
			if (cj == ci)
				continue;
			const std::vector<int> &ocells = scopeCells[cj];
			for (std::size_t zi = 0; zi < ocells.size(); zi++) {
				if (ocells[zi] == cell)
					continue;
				int arc = arcBase[cj] + static_cast<int>(zi);
				if (!inQueue[arc]) {
					inQueue[arc] = 1;
					q.push(arc);
				}
			}
		}
	}
	return true;
}

template <int Width, int Height> int PipsAI<Width, Height>::MRV() const {
	int minCell = -1;
	int minSize = INT_MAX;
	for (int c = 0; c < N; c++) {
		if (!state.inPlay[c] || state.assigned[c])
			continue;
		int sz = state.domain[c].size();
		if (sz < minSize) {
			minSize = sz;
			minCell = c;
		}
	}
	return minCell;
}

template <int Width, int Height> bool PipsAI<Width, Height>::solved() const {
	// Full coverage: every in-play cell fixed and assigned.
	for (int c = 0; c < N; c++) {
		if (!state.inPlay[c])
			continue;
		if (state.domain[c].size() != 1 || !state.assigned[c])
			return false;
	}
	// Every constraint satisfied over the final pip values.
	for (const auto &constraint : constraints) {
		const std::vector<Position> &positions = constraint->getPositions();
		std::vector<int> values;
		values.reserve(positions.size());
		for (const auto &p : positions) {
			int cell = p.row * Width + p.col;
			TileId id = static_cast<TileId>(state.domain[cell].first());
			values.push_back(valueOf[id]);
		}
		if (!constraint->evaluate(std::span<int>(values)))
			return false;
	}
	return true;
}

template <int Width, int Height> bool PipsAI<Width, Height>::popState() {
	while (!frontier.empty()) {
		Branch b = frontier.back();
		frontier.pop_back();
		undoTo(b.trailMark, b.assignMark);
		if (assignVariable(b.cell, b.value))
			return true;
		// dead branch: try the next one.
	}
	return false;
}

template <int Width, int Height>
std::optional<SolverState<Width, Height>> PipsAI<Width, Height>::solve() {

	auto t = std::chrono::high_resolution_clock::now();
	std::cout << "initializing domains and search tree...\n";
	initializeDomains();
	auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now() - t);

	t = std::chrono::high_resolution_clock::now();
	std::cout << "removing constraint breaking values and choosing where to "
				 "being search: \n";
	impossibleValuesPass();
	{
		int pos = MRV();
		if (pos != -1) {
			uint32_t tMark = static_cast<uint32_t>(trail.size());
			uint32_t aMark = static_cast<uint32_t>(assignTrail.size());
			DomainMask copy = state.domain[pos];
			copy.forEach(
				[&](int id) { pushChild(pos, static_cast<TileId>(id), tMark, aMark); });
		}
	}
	auto t2 = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now() - t);

	std::chrono::microseconds t3(0);
	std::vector<std::chrono::microseconds> times{};
	std::cout << "beginning search..." << std::endl;
	while (!frontier.empty()) {
		state.printBoard("");
		auto t = std::chrono::high_resolution_clock::now();
		if (popState() && GAC()) {

			if (solved()) {
				int i = 1;
				for (const auto &time : times) {
					std::cout << "reduction/assignment #" << i
							  << " took: " << time << '\n';
					t3 += time;
					i++;
				}
				std::cout << "total solve time: " << t1 + t2 + t3 << '\n';
				return {state};
			}

			int pos = MRV();
			if (pos != -1) {
				uint32_t tMark = static_cast<uint32_t>(trail.size());
				uint32_t aMark = static_cast<uint32_t>(assignTrail.size());
				DomainMask copy = state.domain[pos];
				copy.forEach([&](int id) {
					pushChild(pos, static_cast<TileId>(id), tMark, aMark);
				});
			}
		}
		times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now() - t));
	}

	return std::nullopt;
}

#endif
