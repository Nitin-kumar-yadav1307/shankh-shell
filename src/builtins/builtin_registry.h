#pragma once

#include <vector>
#include <string>

bool isBuiltin(const std::string& cmd);

void runBuiltin(std::vector<std::string>& toks);