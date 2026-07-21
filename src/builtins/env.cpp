#include "env.h"
#include <iostream>

// OS ka global variable jo saare environment variables store karta hai
extern char **environ; 

void builtinEnv(const std::vector<std::string>& tokens) {
    // Loop through all environment variables and print them
    for (char **env = environ; *env != 0; env++) {
        std::cout << *env << "\n";
    }
}