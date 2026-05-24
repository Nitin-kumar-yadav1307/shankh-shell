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




//helper-> split inputs

char* myCompleter(const char* text , int state){
    static std::vector<std::string> matches ;
    static int index;

    if(state = 0){
        matches.clear();
        index = 0 ;
         for (auto& b : {"echo","exit","pwd","cd","type"}) {
            if (std::string(b).rfind(text, 0) == 0)  // starts with what user typed?
                matches.push_back(b);
        }
    }
        if (index < matches.size())
         return strdup(matches[index++].c_str());
        return nullptr;
}




std::vector<std::string> splitInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken = "";
    bool singleQuoteMode = false;
    bool doubleQuoteMode = false;


    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
           
        if(c == '\\' && !doubleQuoteMode && !singleQuoteMode){
            i++;
            currentToken += input[i];
        }
        else if (c == '\'' && !doubleQuoteMode) {
            singleQuoteMode = !singleQuoteMode;  // toggle quote mode, don't add ' to token
        }
        else if (c == '\"' && !singleQuoteMode) {
            doubleQuoteMode = !doubleQuoteMode;  // toggle quote mode, don't add ' to token
        }
        else if(doubleQuoteMode && c == '\\'){
            char next = input[i+1];
            if(next == '\"' || next == '\\'){
                i++;
                currentToken += input[i];

            }
            else{
                currentToken += '\\';
            }
        }
        else if (c == ' ' && !singleQuoteMode && !doubleQuoteMode) {
            if (!currentToken.empty()) {   // avoid empty tokens from multiple spaces
                tokens.push_back(currentToken);
                currentToken = "";         // reset for next token
            }
        }
        else {
            currentToken += c;             // add character to current token
        }
    }

    if (!currentToken.empty()) {           // save last token
        tokens.push_back(currentToken);
    }

    return tokens;
}

//helper-> find path
std::string findInPath(const std::string& command) {
    char* pathEnv = std::getenv("PATH");
    std::string pathStr(pathEnv ? pathEnv : "");

    size_t start = 0;
    size_t end = pathStr.find(':');

    while (start < pathStr.length()) {
        std::string dir;
        if (end == std::string::npos) {
            dir = pathStr.substr(start);
        } else {
            dir = pathStr.substr(start, end - start);
        }

        std::string fullPath = dir + "/" + command;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && access(fullPath.c_str(), X_OK) == 0) {
            return fullPath;  // found!
        }

        if (end == std::string::npos) break;
        start = end + 1;
        end = pathStr.find(':', start);
    }

    return "";  // not found
}



int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  rl_completion_entry_function = myCompleter;
  // TODO: Uncomment the code below to pass the first stage

  while(true){
   //std::cout << "$ ";
   std::string redirectFile = "";
   //std::string input;
   int indextoken = -1 ;
   int stderrIndexToken = -1;
   std::string stderrRedirectFile = "";
   //std:: getline(std::cin, input);
   char* raw = readline("$ ");
    if (!raw) break;  // EOF
    std::string input(raw);
    free(raw);    
   bool appendMode = false;
   bool stderrAppendMode = false;

  

     if (input.empty()) continue;

        std::vector<std::string> tokens = splitInput(input);
        std::string command = tokens[0];

        for(int i = 0 ; tokens.size()>i ; i++){
           
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
                
            }
           
            if(tokens[i] == ">" || tokens[i] == "1>"){
               appendMode = false;
               indextoken = i ;
               redirectFile = tokens[i+1];
               break;
            }
           
        }

        if(indextoken != -1){
             tokens.erase(tokens.begin() + indextoken+1);  // erase filename
            tokens.erase(tokens.begin() + indextoken);  // erase operator
        }

         if(stderrIndexToken != -1){
             tokens.erase(tokens.begin() + stderrIndexToken+1);  // erase filename
            tokens.erase(tokens.begin() + stderrIndexToken);  // erase operator
        }
    
          
         int savedStdout = -1;
          int fd = -1;
  if(!redirectFile.empty()){
    int flags = O_WRONLY | O_CREAT | (appendMode ? O_APPEND : O_TRUNC);
   fd = open(redirectFile.c_str(), flags, 0644);
    if (fd != -1) {
            savedStdout = dup(1);
            dup2(fd, 1);
            close(fd);
        }
}
       
    int savedStderr = -1;
    if (!stderrRedirectFile.empty()) {
        int stderrFlags = O_WRONLY | O_CREAT | (stderrAppendMode ? O_APPEND : O_TRUNC);
        int stderrFd = open(stderrRedirectFile.c_str(), stderrFlags, 0644);
    if (stderrFd != -1) {
        savedStderr = dup(2);
        dup2(stderrFd, 2);
        close(stderrFd);
    }
}

  
        
 // --- builtins programs
 
   if(command == "exit"){
    break;
   }
   else if(command == "echo"){

    for(size_t i= 1 ; i<tokens.size();i++ ){
      if (i > 1) std::cout << " ";
        std::cout << tokens[i];
    }

     std::cout  << std::endl;
   }
    else if(command == "pwd"){
        
        char buffer[PATH_MAX];    
        if(getcwd(buffer, sizeof(buffer)) != nullptr){
        std::cout << buffer << std::endl;
    } else {
        std::cerr << "pwd: error getting directory" << std::endl;
    }   
   }    

   else if(command == "cd"){
    std::string path = tokens[1];
    if(path == "~"){
        path = getenv("HOME");
    }
    int result =  chdir(path.c_str()) ;
     if(result == -1){
        std::cout<<"cd: "<<path<<": "<<"No such file or directory"<<std::endl;
     } 
   }

    else if (command == "type") {
            if (tokens.size() < 2) continue;
            std::string target = tokens[1];

            if (target == "echo" || target == "exit" || target == "type" || target == "pwd" || target == "cd") {
                std::cout << target << " is a shell builtin" << std::endl;
            } else {
                std::string path = findInPath(target);
                if (!path.empty()) {
                    std::cout << target << " is " << path << std::endl;
                } else {
                    std::cout << target << ": not found" << std::endl;
                }
            }
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
                    int status;
                    wait(&status);
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