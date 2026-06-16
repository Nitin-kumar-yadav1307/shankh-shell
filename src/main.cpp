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
#include <algorithm>





//std::vector<Job> jobs;  // global list of jobs
int nextJobNumber = 1;  // global counter

int lastWrittenIndex = 0;

//helper-> split inputs
//std::map<std::string, std::string> completionSpecs;
//std::map<std::string, std::string> shellVars;




//helper-> find path







bool isBuiltin(const std::string& cmd){
    return cmd=="echo" || cmd=="pwd" || 
           cmd=="type" || cmd=="cd"  ||
           cmd=="exit" || cmd=="jobs"|| 
           cmd=="history" || cmd=="declare";
}




void runBuiltin(std::vector<std::string>& toks){
    if(toks[0] == "echo"){
        for(size_t i = 1; i < toks.size(); i++){
            if(i > 1) std::cout << " ";
            std::cout << toks[i];
        }
        std::cout << "\n";
    }
    else if(toks[0] == "pwd"){
        char buf[PATH_MAX];
        getcwd(buf, sizeof(buf));
        std::cout << buf << "\n";
    }
    else if(toks[0] == "type"){
        std::string target = toks[1];
        if(isBuiltin(target))
            std::cout << target << " is a shell builtin\n";
        else {
            std::string path = findInPath(target);
            if(!path.empty())
                std::cout << target << " is " << path << "\n";
            else
                std::cout << target << ": not found\n";
        }
    }
    else if(toks[0] == "history"){
    HIST_ENTRY** histList = history_list();
    if(histList){
        for(int i = 0; histList[i] != nullptr; i++){
            std::cout << "    " << (i+1) << "  " 
                      << histList[i]->line << "\n";
        }
    }
}
}



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
            // Step 1: split all tokens by |
            std::vector<std::vector<std::string>> commands;
            std::vector<std::string> current;
            for(auto& token : tokens){
                if(token == "|"){
                    commands.push_back(current);
                    current.clear();
                } else {
                    current.push_back(token);
                }
            }
            commands.push_back(current);  // last command

            int numCmds = commands.size();
            int numPipes = numCmds - 1;

            // Step 2: create all pipes
            std::vector<std::array<int,2>> pipes(numPipes);
            for(int i = 0; i < numPipes; i++){
                pipe(pipes[i].data());
            }

            // Step 3: fork each command
            std::vector<pid_t> pids;
            for(int i = 0; i < numCmds; i++){
                pid_t pid = fork();

                if(pid == 0){
                    // if not first → read from previous pipe
                    if(i > 0){
                        dup2(pipes[i-1][0], 0);
                    }
                    // if not last → write to next pipe
                    if(i < numCmds - 1){
                        dup2(pipes[i][1], 1);
                    }
                    // close ALL pipe ends
                    for(int j = 0; j < numPipes; j++){
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    // run command
                    if(isBuiltin(commands[i][0])){
                        runBuiltin(commands[i]);
                        exit(0);
                    } else {
                        std::string path = findInPath(commands[i][0]);
                        std::vector<char*> argv;
                        for(auto& t : commands[i])
                            argv.push_back(const_cast<char*>(t.c_str()));
                        argv.push_back(nullptr);
                        execv(path.c_str(), argv.data());
                        exit(1);
                    }
                }
                pids.push_back(pid);
            }

            // Step 4: parent closes ALL pipes
            for(int i = 0; i < numPipes; i++){
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            // Step 5: wait for ALL children
            for(pid_t p : pids){
                waitpid(p, nullptr, 0);
            }

            continue;  // skip rest of loop
        }
        else{
            
        if(indextoken != -1){
             tokens.erase(tokens.begin() + indextoken+1);  // erase filename
            tokens.erase(tokens.begin() + indextoken);  // erase operator
        }

         if(stderrIndexToken != -1){
             tokens.erase(tokens.begin() + stderrIndexToken+1);  // erase filename
            tokens.erase(tokens.begin() + stderrIndexToken);  // erase operator
        }

         
  if(!redirectFile.empty()){
    int flags = O_WRONLY | O_CREAT | (appendMode ? O_APPEND : O_TRUNC);
   fd = open(redirectFile.c_str(), flags, 0644);
    if (fd != -1) {
            savedStdout = dup(1);
            dup2(fd, 1);
            close(fd);
        }
}
       
    
    if (!stderrRedirectFile.empty()) {
        int stderrFlags = O_WRONLY | O_CREAT | (stderrAppendMode ? O_APPEND : O_TRUNC);
        int stderrFd = open(stderrRedirectFile.c_str(), stderrFlags, 0644);
    if (stderrFd != -1) {
        savedStderr = dup(2);
        dup2(stderrFd, 2);
        close(stderrFd);
    }
}

  

        }

        
 // --- builtins programs
 
   if(command == "exit"){
    char* histFile = getenv("HISTFILE");
    if(histFile){
        write_history(histFile);
    }
    break;
   }
  else if(command == "echo"){
    builtinEcho(tokens);
    }
    else if(command == "pwd"){
         builtinPwd();
       
    } 
 else if(command == "cd"){
    std::string path = "~";

    if(tokens.size() > 1)
    {
        path = tokens[1];
    }

    builtinCd(path);
}

    else if (command == "type") {
             builtinType(tokens);
        }
        else if(command == "complete"){
    if(tokens[1] == "-C" && tokens.size() >= 4){
        // tokens[2] = path, tokens[3] = command
        completionSpecs[tokens[3]] = tokens[2];  // store
        // no output
    }
    else if(tokens[1] == "-p" && tokens.size() >= 3){
        std::string cmd = tokens[2];
        if(completionSpecs.count(cmd) > 0){
            // found → print it
            std::cout << "complete -C '" << completionSpecs[cmd] << "' " << cmd << std::endl;
        } else {
            // not found 
            std::cout << "complete: " << cmd << ": no completion specification" << std::endl;
        }
    }
     else if(tokens[1] == "-r" && tokens.size() >= 3){  
        completionSpecs.erase(tokens[2]);               // ← removes from map
    }
    
    }
     else if(command == "jobs"){
    // check status of all jobs first
    printJobs();
   
    }

       else if(command == "history"){

         // handle -r flag
         if(tokens.size() >= 3 && tokens[1] == "-r"){
        std::string path = tokens[2];
        read_history(path.c_str());  // ← reads file into history
    }
     // handle -w flag (write to file)
    else if(tokens.size() >= 3 && tokens[1] == "-w"){
        std::string path = tokens[2];
        write_history(path.c_str());  // ← write in file by taking history from memory!
    }
    else if(tokens.size() >= 3 && tokens[1] == "-a"){
    std::string path = tokens[2];
    
    // open file in append mode
    FILE* f = fopen(path.c_str(), "a");
    if(f){
        HIST_ENTRY** histList = history_list();
        if(histList){
            // count total
            int total = 0;
            while(histList[total] != nullptr) total++;
            
            // write only new commands
            for(int i = lastWrittenIndex; i < total; i++){
                fprintf(f, "%s\n", histList[i]->line);
            }
            
            // update last written position
            lastWrittenIndex = total;
        }
        fclose(f);
    }
}
    else{
        HIST_ENTRY** histList = history_list();
    if(histList){
        // count total
        int total = 0;
        while(histList[total] != nullptr) total++;

        // find start point
        int start = 0;
        if(tokens.size() >= 2){
            int n = std::stoi(tokens[1]);
            start = total - n;
            if(start < 0) start = 0;
        }

        // print
        for(int i = start; i < total; i++){
            std::cout << "    " << (i+1) << "  "
                      << histList[i]->line << "\n";
        }
    }

    }
    
}
   else if(command == "declare"){
       builtinDeclare(tokens);
}


        // --- External programs ---
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