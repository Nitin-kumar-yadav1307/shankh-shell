#include <cstdlib>  
#include <iostream>
#include<climits>
#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>   // for DIR, opendir, readdir, closedir
#include <map> 
#include <array>  // ← needed for std::array
#include "jobs/job_manager.h"
#include "completion/completion.h"
#include <algorithm>
#include "shell.h"
//std::vector<Job> jobs;  // global list of jobs
int nextJobNumber = 1;  // global counter

int lastWrittenIndex = 0;

//helper-> split inputs
//std::map<std::string, std::string> completionSpecs;
//std::map<std::string, std::string> shellVars;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  rl_completion_entry_function = myCompleter;
  rl_completion_append_character = ' '; 
  // TODO: Uncomment the code below to pass the first stage

   signal(SIGCHLD, [](int) {
    int saved = errno;
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        // find job and mark as done
        for(auto& job : jobs){
            if(job.pid == pid){
                job.status = "Done";
            }
        }
    }
    errno = saved;
});

  char* histFile = getenv("HISTFILE");
    if(histFile){
        read_history(histFile);  // load from file
    }

  while(true)
{
    reapJobs();

    char* raw = readline("$ ");

    if(!raw)
        break;

    std::string input(raw);

    add_history(raw);

    free(raw);

    executeCommand(input);
}
}