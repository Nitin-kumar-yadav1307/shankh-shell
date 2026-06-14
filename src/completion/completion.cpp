#include "completion.h"

#include <readline/readline.h>
#include <readline/history.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


std::map<std::string, std::string> completionSpecs;

char* myCompleter(const char* text , int state){
    static std::vector<std::string> matches ;
    static int index;
     

if(state == 0){
    matches.clear();
        index = 0 ;
        std::string line(rl_line_buffer);
         
     if(line.find(' ') == std::string::npos){
           for (auto& b : {"echo","exit","pwd","cd","type","complete","jobs","history","declare"}) {
            if (std::string(b).rfind(text, 0) == 0)  // starts with what user typed?
                matches.push_back(b);
        }

        // Get PATH directories
    char* pathEnv = std::getenv("PATH");
    std::string pathStr(pathEnv ? pathEnv : "");

    size_t start = 0;
    size_t end = pathStr.find(':');

    while(start < pathStr.length()){
    std::string dir;
        if(end == std::string::npos)
        dir = pathStr.substr(start);
        else
        dir = pathStr.substr(start, end - start);

        // Open the directory
        DIR* d = opendir(dir.c_str());
        if(d){  // directory exists
        struct dirent* entry;
        while((entry = readdir(d)) != nullptr){
            std::string filename(entry->d_name);
            // check if filename starts with text
            if(filename.rfind(text, 0) == 0){
                // check if it's executable
                std::string fullPath = dir + "/" + filename;
                if(access(fullPath.c_str(), X_OK) == 0)
                    matches.push_back(filename);
            }
        }
        closedir(d);
     }

         if(end == std::string::npos) break;
        start = end + 1;
        end = pathStr.find(':', start);
     }

         if(matches.empty()){
        std::cout << "\x07";
        std::cout.flush();
        }

        } else {


    // extract command name (first word)
  
    std::string cmdName = line.substr(0, line.find(' '));

    // check if completer registered
    if(completionSpecs.count(cmdName) > 0){
        std::string scriptPath = completionSpecs[cmdName];
        // split line into words
    std::vector<std::string> lineWords;
    std::istringstream ss(line);
    std::string w;
    while(ss >> w) lineWords.push_back(w);

    // get previous word
    std::string prevWord = "";
    if(lineWords.size() >= 2){
    prevWord = lineWords[lineWords.size() - 2];
    }

        // run script and read output
        int fd[2];
        pipe(fd);
        pid_t pid = fork();

        if(pid == 0){
            // set environment variables
             setenv("COMP_LINE", line.c_str(), 1);
            setenv("COMP_POINT", std::to_string(line.length()).c_str(), 1);
            // child
            close(fd[0]);
            dup2(fd[1], 1);   // stdout → pipe
            close(fd[1]);
           execl(scriptPath.c_str(), 
                scriptPath.c_str(),   // argv[0] = program name
                cmdName.c_str(),      // argv[1] = command name
                text,                  // argv[2] = partial text
                prevWord.c_str(),                
                nullptr);
            exit(1);
        } else {
            // parent reads output
            close(fd[1]);
            char buffer[1024];
            std::string output = "";
            int bytes;
            while((bytes = read(fd[0], buffer, sizeof(buffer)-1)) > 0){
                buffer[bytes] = '\0';
                output += buffer;
            }
            close(fd[0]);
            wait(nullptr);

            // each line is a completion candidate
            std::istringstream stream(output);
            std::string word;
            while(std::getline(stream, word)){
                if(!word.empty())
                    matches.push_back(word);
            }
        }
    }
         if(matches.empty()){
    //  filename completion code
            // opendir(".") + readdir + rfind check
            std::string textStr(text);// just conversion of char* from string
            size_t lastSlash = textStr.rfind('/'); // rfind find tha lastone of that char in string
            if(lastSlash != std::string::npos){ // npos->not found
                std::string directory = textStr.substr(0,lastSlash+1);
                std::string prefix  = textStr.substr(lastSlash+1);

                 DIR* d = opendir(directory.c_str());
            if(d){
                struct dirent* entry;
                while((entry = readdir(d)) != nullptr){
                        std::string filename(entry->d_name);
                        if(filename == "." || filename == "..") continue;
                    if(filename.rfind(prefix, 0) == 0){
                  std::string fullPath = directory + filename;
                    struct stat st;
                    stat(fullPath.c_str(), &st);
                    if(S_ISDIR(st.st_mode)){//S_ISDIR is a macro that checks if something is a directory.
                        matches.push_back(directory+filename + "/");
                        rl_completion_append_character = '\0';
                    } else {
                                matches.push_back(directory+filename);
                            rl_completion_append_character = ' ';
                    } // ← full path!
            }
        }
        closedir(d);
    }

            }
            else {
                DIR* d = opendir(".");
                 if(d){
                    struct dirent* entry;
                    while((entry = readdir(d)) != nullptr){
                        std::string filename(entry->d_name);
                          if(filename == "." || filename == "..") continue;
                        if(filename.rfind(text,0) == 0){
                       std::string fullPath = "./" + filename;
                       struct stat st;
                        stat(fullPath.c_str(), &st);
                        if(S_ISDIR(st.st_mode)){
                                matches.push_back( filename + "/");
                                rl_completion_append_character = '\0';
                        } else {
                            matches.push_back( filename);
                            rl_completion_append_character = ' ';
                        }

                    }
                } 
                closedir(d);
        }
            }
}
            
         
        }
       
    }
   
        if (index < matches.size())
         return strdup(matches[index++].c_str());
        return nullptr;
}