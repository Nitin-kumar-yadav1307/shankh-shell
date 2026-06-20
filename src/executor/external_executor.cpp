#include "external_executor.h"

#include "../utils/path_utils.h"
#include "redirection.h"
#include "../jobs/job_manager.h"

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void executeExternal(
    const std::vector<std::string>& tokens,
    bool background,
    const std::string& redirectFile,
    bool appendMode,
    const std::string& stderrRedirectFile,
    bool stderrAppendMode
)
{
    std::string fullPath = findInPath(tokens[0]);

    if(fullPath.empty())
    {
        std::cout << tokens[0]
                  << ": command not found\n";
        return;
    }

    std::vector<char*> argv;

    for(auto& token : tokens)
    {
        argv.push_back(
            const_cast<char*>(token.c_str())
        );
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if(pid == 0)
    {
        RedirectionContext ctx;

        applyRedirections(
            redirectFile,
            appendMode,
            stderrRedirectFile,
            stderrAppendMode,
            ctx
        );

        execv(
            fullPath.c_str(),
            argv.data()
        );

        std::cerr << "execv failed\n";
        exit(1);
    }
    else if(pid > 0)
    {
        if(background)
        {
            std::string cmdStr;

            for(size_t i = 0; i < tokens.size(); i++)
            {
                if(i > 0)
                    cmdStr += " ";

                cmdStr += tokens[i];
            }

            cmdStr += " &";

            int jobNumber =
                addJob(pid, cmdStr);

            std::cout
                << "[" << jobNumber
                << "] "
                << pid
                << "\n";
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }
    else
    {
        std::cerr
            << "fork failed\n";
    }
}