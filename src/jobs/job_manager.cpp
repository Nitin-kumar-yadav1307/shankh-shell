#include <iostream>
#include "job_manager.h"
#include <algorithm>
#include <sys/wait.h>

std::vector<Job> jobs;

void reapJobs(){
    // first mark exited jobs
    for(auto& job : jobs){
        int status;
        pid_t result = waitpid(job.pid, &status, WNOHANG);
        if(result == job.pid && WIFEXITED(status)){
            job.status = "Done";
        }
    }

    int total = jobs.size();
    for(int i = 0; i < total; i++){
        if(jobs[i].status == "Done"){
            char marker;
            if(i == total - 1)      marker = '+';
            else if(i == total - 2) marker = '-';
            else                    marker = ' ';

            std::string cmd = jobs[i].command;
            if(cmd.size() >= 2 && cmd.substr(cmd.size()-2) == " &")
                cmd = cmd.substr(0, cmd.size()-2);

            std::string status = "Done";
            while(status.length() < 24) status += " ";  // ← 24

            std::cout << "[" << jobs[i].number << "]"
                      << marker << "  "
                      << status
                      << cmd << "\n";
        }
    }

    jobs.erase(
        std::remove_if(jobs.begin(), jobs.end(),
            [](const Job& j){ return j.status == "Done"; }),
        jobs.end()
    );
}


    int addJob(pid_t pid, const std::string& command)
    {
    int newNumber = 1;
    while(true){
        bool taken = false;

        for(auto& j : jobs){
            if(j.number == newNumber){
                taken = true;
                break;
            }
        }

        if(!taken) break;
        newNumber++;
    }

    Job newJob = {newNumber, pid, command, "Running"};
    jobs.push_back(newJob);

    return newJob.number;

}

void printJobs()
{
     for(auto& job : jobs){
        int status;
        pid_t result = waitpid(job.pid, &status, WNOHANG);
        if(result == job.pid && WIFEXITED(status)){
            job.status = "Done";
        }
    }

    // print ALL jobs in order (Running and Done together)
    int total = jobs.size();
    for(int i = 0; i < total; i++){
        auto& job = jobs[i];

        char marker;
        if(i == total - 1)       marker = '+';
        else if(i == total - 2)  marker = '-';
        else                     marker = ' ';

        std::string cmd = job.command;
        if(job.status == "Done" && cmd.size() >= 2 
           && cmd.substr(cmd.size()-2) == " &")
            cmd = cmd.substr(0, cmd.size()-2);

        std::string status = job.status;
        while(status.length() < 24) status += " ";

        std::cout << "[" << job.number << "]"
                  << marker << "  "
                  << status
                  << cmd << "\n";
    }

    // remove Done jobs after printing
    jobs.erase(
        std::remove_if(jobs.begin(), jobs.end(),
            [](const Job& j){ return j.status == "Done"; }),
        jobs.end()
    );
}
    