#include "helpers.hpp"
#include <map>
#include <set>
#include <stdexcept>

int findMode(std::initializer_list<int> values) {
  if (values.size() == 0)
    throw std::runtime_error("Empty array has no mode.");
  std::map<int, int> counts{};
  for (const auto &num : values) {
    counts[num] += 1;
  }

  int mode, modeCount = 0;
  for (const auto &[key, value] : counts) {
    if (value > modeCount) {
      mode = key, modeCount = value;
    }
  }
  return mode;
}

int findRepeatCount(std::initializer_list<int> values) {
	std::set<int> seen{};
	int count = 0;
	for(const auto &num : values){
		if(seen.find(num) == seen.end()) seen.insert(num);
		else count++;
	}
	return count;
}
