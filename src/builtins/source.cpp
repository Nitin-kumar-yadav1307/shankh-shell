#include "source.h"

#include "../shell.h"

#include <fstream>
#include <iostream>
#include <string>

void builtinSource(const std::vector<std::string>& tokens)
{
    if(tokens.size() < 2)
    {
        std::cout << "source: filename required\n";
        return;
    }

    std::ifstream file(tokens[1]);

    if(!file)
    {
        std::cout << "source: cannot open " << tokens[1] << "\n";
        return;
    }

    std::string line;

    while(std::getline(file, line))
    {
        if(line.empty())
            continue;

        executeCommand(line);
    }
}