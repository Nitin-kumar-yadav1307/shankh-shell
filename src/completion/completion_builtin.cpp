#include "completion_builtin.h"
#include "completion.h"

#include <iostream>

void builtinComplete(const std::vector<std::string>& tokens)
{
    if(tokens.size() < 2)
        return;

    if(tokens[1] == "-C" && tokens.size() >= 4)
    {
        completionSpecs[tokens[3]] = tokens[2];
    }
    else if(tokens[1] == "-p" && tokens.size() >= 3)
    {
        std::string cmd = tokens[2];

        if(completionSpecs.count(cmd) > 0)
        {
            std::cout << "complete -C '"
                      << completionSpecs[cmd]
                      << "' "
                      << cmd
                      << std::endl;
        }
        else
        {
            std::cout << "complete: "
                      << cmd
                      << ": no completion specification"
                      << std::endl;
        }
    }
    else if(tokens[1] == "-r" && tokens.size() >= 3)
    {
        completionSpecs.erase(tokens[2]);
    }
}