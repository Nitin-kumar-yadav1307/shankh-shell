#include "history.h"

#include <iostream>
#include <cstdio>
#include <readline/history.h>

void builtinHistory(const std::vector<std::string>& tokens)
{
    // history -r file
    if(tokens.size() >= 3 && tokens[1] == "-r")
    {
        std::string path = tokens[2];
        read_history(path.c_str());
    }

    // history -w file
    else if(tokens.size() >= 3 && tokens[1] == "-w")
    {
        std::string path = tokens[2];
        write_history(path.c_str());
    }

    // history -a file
    else if(tokens.size() >= 3 && tokens[1] == "-a")
    {
        std::string path = tokens[2];

        FILE* f = fopen(path.c_str(), "a");

        if(f)
        {
            HIST_ENTRY** histList = history_list();

            if(histList)
            {
                int total = 0;
                while(histList[total] != nullptr)
                    total++;

                for(int i = lastWrittenIndex; i < total; i++)
                {
                    fprintf(f, "%s\n", histList[i]->line);
                }

                lastWrittenIndex = total;
            }

            fclose(f);
        }
    }

    // history / history N
    else
    {
        HIST_ENTRY** histList = history_list();

        if(histList)
        {
            int total = 0;
            while(histList[total] != nullptr)
                total++;

            int start = 0;

            if(tokens.size() >= 2)
            {
                try
                {
                    int n = std::stoi(tokens[1]);

                    start = total - n;

                    if(start < 0)
                        start = 0;
                }
                catch(...)
                {
                    std::cout << "history: invalid argument\n";
                    return;
                }
            }

            for(int i = start; i < total; i++)
            {
                std::cout << "    "
                          << (i + 1)
                          << "  "
                          << histList[i]->line
                          << "\n";
            }
        }
    }
}