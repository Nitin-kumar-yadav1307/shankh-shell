#include "unset.h"
#include <iostream>
#include <cstdlib> // unsetenv ke liye zaroori hai

void builtinUnset(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cerr << "unset: not enough arguments\n";
        return;
    }
    
    // Multiple variables ek sath unset karne ka support (e.g., unset VAR1 VAR2)
    for (size_t i = 1; i < tokens.size(); i++) {
        if (unsetenv(tokens[i].c_str()) != 0) {
            perror("unset"); // Agar koi error aaye
        }
    }
}