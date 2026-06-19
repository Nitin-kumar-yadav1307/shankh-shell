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
#include "parser/parser.h"
#include "utils/path_utils.h"
#include "variables/variable_manager.h"
#include "jobs/job_manager.h"
#include "completion/completion.h"
#include "builtins/pwd.h"
#include "builtins/cd.h"
#include "builtins/echo.h"
#include "builtins/type.h"
#include "builtins/history.h"
#include "builtins/builtin_registry.h"
#include "executor/pipeline_executor.h"
#include <algorithm>





//std::vector<Job> jobs;  // global list of jobs
int nextJobNumber = 1;  // global counter

int lastWrittenIndex = 0;

//helper-> split inputs
//std::map<std::string, std::string> completionSpecs;
//std::map<std::string, std::string> shellVars;




//helper-> find path















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

  while(true){
   //std::cout << "$ ";
   std::string redirectFile = "";
   //std::string input;
   int indextoken = -1 ;
   int pipeIndex = -1;
   int stderrIndexToken = -1;
   std::string stderrRedirectFile = "";
   //std:: getline(std::cin, input);
   reapJobs();  
    char* raw = readline("$ ");  // empty string since we already printed prompt
    if (!raw){ // EOF
        char* histFile = getenv("HISTFILE");
        if(histFile) write_history(histFile);
      break;
    }   
    std::string input(raw);
    add_history(raw); 
    free(raw);    
   bool appendMode = false;
   bool stderrAppendMode = false;

  

     if (input.empty()) continue;

        std::vector<std::string> tokens = splitInput(input);
       

        // expand variables
        for(auto& token : tokens){
            if(token != "|" && token != ">" && token != ">>" &&
            token != "2>" && token != "2>>" && token != "&" &&
            token != "1>" && token != "1>>"){
                token = expandVariables(token);
            }
        }

        // check for background
        bool background = false;
        if(!tokens.empty() && tokens.back() == "&"){
        background = true;
        tokens.pop_back();  // remove &
    }
        // also trim any empty tokens left behind
        while(!tokens.empty() && tokens.back().empty()){
        tokens.pop_back();
    }
        std::string command = tokens[0];

        for(int i = 0 ; tokens.size()>i ; i++){

             if(tokens[i] == "|"){
                pipeIndex = i ;
                break;
            }
           
           
            if(tokens[i] == "2>>"){
                stderrAppendMode = true;
                stderrIndexToken = i;
                stderrRedirectFile = tokens[i+1];
                break;
            }


             if(tokens[i] == "2>"){
                stderrAppendMode = false;
                stderrIndexToken = i;
                stderrRedirectFile = tokens[i+1];
                break;
            }

            if(tokens[i] == ">>" || tokens[i] == "1>>"){
                appendMode = true;
                indextoken = i ;
               redirectFile = tokens[i+1];
                break;
            }
           
            if(tokens[i] == ">" || tokens[i] == "1>"){
               appendMode = false;
               indextoken = i ;
               redirectFile = tokens[i+1];
               break;
            }

           
        }

        int savedStdout = -1;
        int savedStderr = -1;
          int fd = -1;

        if(pipeIndex != -1){
             executePipeline(tokens);
            continue;

        }

        
 // --- builtins programs
 
   if(command == "exit"){
    char* histFile = getenv("HISTFILE");
    if(histFile){
        write_history(histFile);
    }
    break;
   }
       
     else if(isBuiltin(command)){
    runBuiltin(tokens);
    }

        else {
            std::string fullPath = findInPath(command);

            if (fullPath.empty()) {
                std::cout << command << ": command not found" << std::endl;
            } else {
                // Build argv array for execv
                // execv needs: char* argv[] = { program, arg1, arg2, ..., NULL }
                std::vector<char*> argv;
                for (auto& token : tokens) {
                    argv.push_back(const_cast<char*>(token.c_str()));
                }
                argv.push_back(nullptr);  // must be null-terminated

                pid_t pid = fork();

                if (pid == 0) {

                  if (!redirectFile.empty()) {
                        int stdoutFlags = O_WRONLY | O_CREAT | (appendMode ? O_APPEND : O_TRUNC);
                        int stdoutFd = open(redirectFile.c_str(), stdoutFlags, 0644);
                        dup2(stdoutFd, 1);
                         close(stdoutFd);
                        }

                   if (!stderrRedirectFile.empty()) {
                        int stderrFlags = O_WRONLY | O_CREAT | (stderrAppendMode ? O_APPEND : O_TRUNC);
                        int stderrFd = open(stderrRedirectFile.c_str(), stderrFlags, 0644);
                        dup2(stderrFd, 2);
                        close(stderrFd);
                    }
                    // Child process: replace itself with the program
                    execv(fullPath.c_str(), argv.data());
                    // execv only returns if it FAILED
                    std::cerr << "execv failed" << std::endl;
                    exit(1);
                } else if (pid > 0) {
                    // Parent process: wait for child to finish
                   
                   if (background) {
                      // build command string
                         std::string cmdStr = "";
                        for(size_t i = 0; i < tokens.size(); i++){
                         if(i > 0) cmdStr += " ";
                        cmdStr += tokens[i];
                        }
                        cmdStr += " &";

                        

                        // store job first
                    // find smallest available job number
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

                    int jobNumber = addJob(pid, cmdStr);
                    // then print using the stored job number
                    std::cout << "[" << jobNumber << "] " << pid << "\n";
                    std::cout.flush();

                   // nextJobNumber++;  // increment for next job
                    }
                   else{
                        int status;
                        waitpid(pid, &status, 0);
                   }
                   
                } else {
                    std::cerr << "fork failed" << std::endl;
                   
                }
            }
        }

        if(savedStdout != -1){
            dup2(savedStdout, 1);
            close(savedStdout);
        }
        if (savedStderr != -1) {
       dup2(savedStderr, 2);
       close(savedStderr);
       }
    }
}