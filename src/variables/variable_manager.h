#pragma once

#include <map>
#include <string>

extern std::map<std::string, std::string> shellVars;

bool isValidIdentifier(const std::string& name);

std::string expandVariables(const std::string& token);