#pragma once

#include <string>
#include <vector> // Required for std::vector

std::string findInPath(const std::string& command);

// NEW: Function to expand wildcard characters into actual file paths
void expandGlobs(std::vector<std::string>& tokens);