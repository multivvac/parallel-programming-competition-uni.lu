#pragma once

#include <iostream>
#include <string>
#include <vector>

std::pair<int, int> compute(
  std::vector<int>& art_data,
  int art_width_per_line,
  std::vector<int>& pattern_data,
  int pattern_width_per_line
);
