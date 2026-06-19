#include "pipeline_executor.h"

#include "../builtins/builtin_registry.h"
#include "../utils/path_utils.h"

#include <array>
#include <vector>
#include <string>

#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

void executePipeline(const std::vector<std::string>& tokens)
{
    // Step 1: split commands by |
    std::vector<std::vector<std::string>> commands;
    std::vector<std::string> current;

    for(const auto& token : tokens)
    {
        if(token == "|")
        {
            commands.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(token);
        }
    }

    commands.push_back(current);

    int numCmds = commands.size();
    int numPipes = numCmds - 1;

    // Step 2: create pipes
    std::vector<std::array<int, 2>> pipes(numPipes);

    for(int i = 0; i < numPipes; i++)
    {
        pipe(pipes[i].data());
    }

    // Step 3: fork processes
    std::vector<pid_t> pids;

    for(int i = 0; i < numCmds; i++)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            // connect stdin
            if(i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // connect stdout
            if(i < numCmds - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // close all pipe ends
            for(int j = 0; j < numPipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // builtin command
            if(isBuiltin(commands[i][0]))
            {
                runBuiltin(commands[i]);
                exit(0);
            }

            // external command
            std::string path = findInPath(commands[i][0]);

            if(path.empty())
            {
                exit(1);
            }

            std::vector<char*> argv;

            for(auto& arg : commands[i])
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }

            argv.push_back(nullptr);

            execv(path.c_str(), argv.data());

            exit(1);
        }

        pids.push_back(pid);
    }

    // Step 4: close all pipe ends in parent
    for(int i = 0; i < numPipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Step 5: wait for all children
    for(pid_t pid : pids)
    {
        waitpid(pid, nullptr, 0);
    }
}