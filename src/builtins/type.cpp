#include "type.h"

#include "../utils/path_utils.h"

#include <iostream>

void builtinType(const std::vector<std::string>& tokens){
   if (tokens.size() < 2) return;
            std::string target = tokens[1];

            if (target == "echo" || 
                target == "exit" || 
                target == "type" || 
                target == "pwd"  || 
                target == "cd"   ||
           target == "complete"  || 
                target == "jobs" || 
             target == "history" || 
            target == "declare") {
                
                std::cout << target << " is a shell builtin" << std::endl;
            } else {
                std::string path = findInPath(target);
                if (!path.empty()) {
                    std::cout << target << " is " << path << std::endl;
                } else {
                    std::cout << target << ": not found" << std::endl;
                }
            }
}