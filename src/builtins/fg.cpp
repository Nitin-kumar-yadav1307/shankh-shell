#include "fg.h"

#include "../jobs/job_manager.h"

#include <iostream>
#include <sys/wait.h>
#include <signal.h>

void builtinFg(const std::vector<std::string>& tokens)
{
    Job* job = nullptr;

    if(tokens.size() == 1)
    {
        job = getLastJob();
    }
    else
    {
        std::string arg = tokens[1];

        if(arg[0] == '%')
            arg.erase(0,1);

        job = getJobByNumber(std::stoi(arg));
    }

    if(job == nullptr)
    {
        std::cout << "fg: no such job\n";
        return;
    }

    kill(job->pid, SIGCONT);

    std::string cmd = job->command;

    if(cmd.size() >= 2 && cmd.substr(cmd.size() - 2) == " &")
    {
        cmd = cmd.substr(0, cmd.size() - 2);
    }

    std::cout << cmd << std::endl;
        

    waitpid(job->pid, nullptr, 0);

    removeJob(job->pid);
}