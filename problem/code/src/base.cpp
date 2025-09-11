#include "base.hpp"
#include <sstream>
#include <stdexcept>
#include <limits>

inline int& at(std::vector<int>& data, int row, int col, int width) {
    return data[row * width + col];
}

std::pair<int, int> compute(
  std::vector<int>& art_data, int art_width,
  std::vector<int>& pattern_data, int pattern_width
) {
  return std::pair<int, int>(-1, -1);
}
