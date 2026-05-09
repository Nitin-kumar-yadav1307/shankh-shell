#include <iostream>
#include<climits>
#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


//helper-> split inputs
std::vector<std::string> splitInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken = "";
    bool quoteMode = false;
    bool doubleQuoteMode = false;


    for (int i = 0; i < input.length(); i++) {
        char c = input[i];

        if (c == '\'' || c == '\"') {
            quoteMode = !quoteMode;
              doubleQuoteMode = !doubleQuoteMode;      // toggle quote mode, don't add ' to token
        }
       
        else if (c == ' ' && !quoteMode && !doubleQuoteMode) {
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

  // TODO: Uncomment the code below to pass the first stage

  while(true){
   std::cout << "$ ";

   std::string input;
   std:: getline(std::cin, input);

     if (input.empty()) continue;

        std::vector<std::string> tokens = splitInput(input);
        std::string command = tokens[0];

        
 // --- builtins programs
   if(command == "exit"){
    break;
   }
   if(command == "echo "){

    for(size_t i= 0 ; i<tokens.size();i++ ){
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

            if (target == "echo" || target == "exit" || target == "type" || target == "pwd") {
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
    }
}