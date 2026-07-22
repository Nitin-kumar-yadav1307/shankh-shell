#include "read.h"
#include <iostream>
#include <string>
#include <cstdlib>

void builtinRead(const std::vector<std::string>& tokens) {
    // STEP 3: Edge Case (Defensive programming - variable name missing)
    if (tokens.size() < 2) {
        std::cerr << "read: missing variable name\n";
        return;
    }

    // STEP 1: Understanding the requirement (Extract variable name)
    std::string variableName = tokens[1];
    std::string userInput;

    // STEP 2: OS Interaction (Wait for input from STDIN)
    // STEP 3: Defensive (if getline fails due to Ctrl+D, it skips safely)
    if (std::getline(std::cin, userInput)) {
        
        // STEP 2: OS Interaction (Save to memory using setenv)
        // 1 means overwrite if variable already exists
        if (setenv(variableName.c_str(), userInput.c_str(), 1) != 0) {
            perror("read"); // OS level error aane par print karo
        }
    }
}