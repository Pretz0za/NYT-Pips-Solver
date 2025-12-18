#include "pips/PipsSolver.hpp"
#include <stdexcept>

Variable::Variable(bool active) : inPlay(active) {
	domain = {};
	reductions = {};
}

Variable::Variable(Tile tile) : inPlay(tile.inPlay), reductions{}, domain{} {
	if (tile.inPlay)
		domain.insert(tile);
}

Variable &Variable::operator=(const Variable &other) {
	if (inPlay != other.inPlay)
		throw std::runtime_error(
			"Cannot set inPlay Variable equal to not inPlay Variable");
	domain = other.domain;
	reductions = other.reductions;
	return *this;
}

void Variable::insertInDomain(std::set<Tile> &values) { domain.merge(values); }

void Variable::pruneDomain(const std::set<Tile> &values) {
	for (const auto &val : values) {
		domain.erase(val);
	}
}

void Variable::pushReduction(const std::set<Tile> &values) {
	reductions.push_back(values);
}

void Variable::assign(const Tile &value) {
	if (domain.find(value) == domain.end())
		throw std::runtime_error("Variable::assign value not in domain");
	domain.erase(value);
	reductions.push_back(domain);
	domain.clear();
	domain.insert(value);
}

void Variable::undo() {
	if (reductions.empty())
		return;
	auto reduction = reductions.back();
	reductions.pop_back();
	domain.merge(reduction);
}
