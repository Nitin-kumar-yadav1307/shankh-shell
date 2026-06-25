#pragma once

#include <string>
#include <sys/types.h>
#include <vector>

struct Job {
    int number;
    pid_t pid;
    std::string command;
    std::string status;
};



extern std::vector<Job> jobs;

void reapJobs();

int addJob(pid_t pid, const std::string& command);

Job* getJobByNumber(int number);

Job* getLastJob();

void removeJob(pid_t pid);

void printJobs();