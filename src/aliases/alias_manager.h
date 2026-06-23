
#pragma once

#include "alias_manager.h"
#include <vector>
#include <string>
#include <map>

extern std::map<std::string, std::string> aliases;

void expandAlias(std::vector<std::string>& tokens);

void addAlias(const std::string& name,const std::string& value);

bool hasAlias(const std::string& name);

std::string getAlias(const std::string& name);

void removeAlias(const std::string& name);