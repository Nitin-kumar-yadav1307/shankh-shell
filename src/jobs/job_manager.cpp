#include <iostream>
#include "job_manager.h"
#include <algorithm>
#include <sys/wait.h>
#include "../builtins/kill.h" // Adjusted path just in case

std::vector<Job> jobs;

void reapJobs(){
    // first mark exited and killed jobs
    for(auto& job : jobs){
        int status;
        pid_t result = waitpid(job.pid, &status, WNOHANG);
        if(result == job.pid) {
            if (WIFEXITED(status)) {
                job.status = "Done";
            } else if (WIFSIGNALED(status)) {
                job.status = "Terminated";
            }
        }
    }

    int total = jobs.size();
    for(int i = 0; i < total; i++){
        if(jobs[i].status == "Done" || jobs[i].status == "Terminated"){
            char marker;
            if(i == total - 1)      marker = '+';
            else if(i == total - 2) marker = '-';
            else                    marker = ' ';

            std::string cmd = jobs[i].command;
            if(cmd.size() >= 2 && cmd.substr(cmd.size()-2) == " &")
                cmd = cmd.substr(0, cmd.size()-2);

            std::string status = jobs[i].status;
            while(status.length() < 24) status += " ";  // ← 24

            std::cout << "[" << jobs[i].number << "]"
                      << marker << "  "
                      << status
                      << cmd << "\n";
        }
    }

    jobs.erase(
        std::remove_if(jobs.begin(), jobs.end(),
            [](const Job& j){ return j.status == "Done" || j.status == "Terminated"; }),
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

Job* getLastJob()
{
    if(jobs.empty())
        return nullptr;

    return &jobs.back();
}

Job* getJobByNumber(int number)
{
    for(auto& job : jobs)
    {
        if(job.number == number)
            return &job;
    }

    return nullptr;
}

void removeJob(pid_t pid)
{
    jobs.erase(
        std::remove_if(
            jobs.begin(),
            jobs.end(),
            [pid](const Job& job)
            {
                return job.pid == pid;
            }),
        jobs.end()
    );
}

void printJobs()
{
     for(auto& job : jobs){
        int status;
        pid_t result = waitpid(job.pid, &status, WNOHANG);
        if(result == job.pid) {
            if (WIFEXITED(status)) {
                job.status = "Done";
            } else if (WIFSIGNALED(status)) {
                job.status = "Terminated";
            }
        }
    }

    // print ALL jobs in order (Running, Done, Terminated together)
    int total = jobs.size();
    for(int i = 0; i < total; i++){
        auto& job = jobs[i];

        char marker;
        if(i == total - 1)       marker = '+';
        else if(i == total - 2)  marker = '-';
        else                     marker = ' ';

        std::string cmd = job.command;
        if((job.status == "Done" || job.status == "Terminated") && cmd.size() >= 2 
           && cmd.substr(cmd.size()-2) == " &")
            cmd = cmd.substr(0, cmd.size()-2);

        std::string status = job.status;
        while(status.length() < 24) status += " ";

        std::cout << "[" << job.number << "]"
                  << marker << "  "
                  << status
                  << cmd << "\n";
    }

    // remove Done and Terminated jobs after printing
    jobs.erase(
        std::remove_if(jobs.begin(), jobs.end(),
            [](const Job& j){ return j.status == "Done" || j.status == "Terminated"; }),
        jobs.end()
    );
}