#pragma once

#include <map>
#include <string>

extern std::map<std::string, std::string> completionSpecs;

char* myCompleter(const char* text,int state);