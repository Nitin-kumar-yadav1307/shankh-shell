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
#include "executor/external_executor.h"
#include "executor/redirection.h"
#include "aliases/alias_manager.h"
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

        // alias expansion
        expandAlias(tokens);
       

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
            executeExternal(
                tokens,
                background,
                redirectFile,
                appendMode,
                stderrRedirectFile,
                stderrAppendMode  );
                           
        }

      
    }
}