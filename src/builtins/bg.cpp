#include "bg.h"

#include "../jobs/job_manager.h"

#include <iostream>
#include <signal.h>

void builtinBg(const std::vector<std::string>& tokens)
{
    Job* job = nullptr;

    if(tokens.size() == 1)
    {
        job = getLastJob();
    }
    else
    {
        std::string arg = tokens[1];

        if(!arg.empty() && arg[0] == '%')
            arg.erase(0, 1);

        job = getJobByNumber(std::stoi(arg));
    }

    if(job == nullptr)
    {
        std::cout << "bg: no such job\n";
        return;
    }

    kill(job->pid, SIGCONT);

    job->status = "Running";

    std::cout << "[" << job->number << "] "
          << job->command
          << std::endl;
}