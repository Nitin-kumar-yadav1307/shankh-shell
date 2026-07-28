#include "cd.h"
#include <iostream>
#include <unistd.h> // chdir() ke liye
#include <cstdlib>  // getenv() aur setenv() ke liye
#include <vector>   
#include <string>   

void builtinCd(const std::vector<std::string>& tokens) {
    std::string path;

    // STEP 1: Defensive Programming - Decide the path
    if (tokens.size() < 2 || tokens[1].empty()) {
        // Agar sirf "cd" likha hai, toh Home directory mein jao
        const char* home = getenv("HOME");
        if (home) {
            path = home;
        } else {
            std::cerr << "cd: HOME not set\n";
            return;
        }
    } else {
        path = tokens[1];
    }

    // STEP 2: OS Interaction - Execute the directory change
    if (chdir(path.c_str()) != 0) {
        // Agar folder nahi mila, ya permission nahi hai
        perror("cd");
        return;
    }

    // STEP 3: Ecosystem Update (Optional but Good Practice)
    // Terminal ko batana padta hai ki PWD (Present Working Directory) change ho gayi hai
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        setenv("PWD", cwd, 1);
    }
}