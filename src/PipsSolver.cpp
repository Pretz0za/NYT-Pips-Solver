#include "pips/PipsSolver.hpp"
#include <memory>
#include <unordered_set>

Variable::Variable(bool inPlay) : active(inPlay), assigned{false}, domain{} {}

Variable::Variable(Tile tile)
	: active(tile.inPlay()), domain{}, assigned(false) {
	if (tile.inPlay())
		domain.insert(tile);
}

Variable &Variable::operator=(const Variable &other) {
	assigned = other.assigned;
	active = other.active;
	domain = other.domain;
	return *this;
}

void Variable::setAssigned() { assigned = true; }
bool Variable::isAssigned() const { return assigned; }

void Variable::insertInDomain(std::unordered_set<Tile> values) {
	domain.merge(values);
}

std::unordered_set<Tile> Variable::clearDomino(std::shared_ptr<Domino> domino) {
	std::unordered_set<Tile> reduction{};
	std::unordered_set<Tile> originalDomain = domain;
	for (const auto &tile : originalDomain) {
		if (tile.getDomino() == domino) {
			reduction.insert(tile);
			domain.erase(tile);
		}
	}
	return reduction;
}

void Variable::pruneDomain(const Tile &value) { domain.erase(value); }
