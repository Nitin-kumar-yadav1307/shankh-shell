#include "kill.h"
#include <iostream>
#include <string>
#include <csignal>     // kill() function aur signals ke liye
#include <sys/types.h>

void builtinKill(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cerr << "kill: usage: kill <pid>\n";
        return;
    }
    
    try {
        // User input se PID extract karna
        pid_t pid = std::stoi(tokens[1]);
        
        // kill(pid, signal) OS ko bolta hai process band karne
        // SIGTERM (15) ek safe termination signal hai
        if (kill(pid, SIGTERM) == -1) {
            perror("kill"); // Agar PID galat hui ya permission nahi mili
        } else {
            std::cout << "[Sent SIGTERM to PID " << pid << "]\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "kill: invalid pid: " << tokens[1] << "\n";
    }
}