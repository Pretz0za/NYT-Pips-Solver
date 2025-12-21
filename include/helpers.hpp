#include <iostream>
#include <string>

inline void printAt(int x, int y, std::string line) {
	std::cout << "\x1b[" << y << ";" << x + 1 << "H" << line;
}
