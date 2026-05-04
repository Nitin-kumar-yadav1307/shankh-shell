#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage

  while(true){
   std::cout << "$ ";

   std::string input;
   std:: getline(std::cin, input);
   if(input == "exit"){
    break;
   }
   if(input.substr(0,5) == "echo "){
     std::cout << input.substr(5) << std::endl;
   }
   else if(input.substr(0,5) == "type "){
      std::string command = input.substr(5);
      if (command == "echo" || command == "exit" || command == "type") {
        std::cout << command << " is a shell builtin" << std::endl;
      }
      else {

          char *pathEnv = std::getenv("PATH");
        std::string pathStr(pathEnv ? pathEnv : "");

        bool found = false;
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
          if (stat(fullPath.c_str(), &st) == 0) {
            if (access(fullPath.c_str(), X_OK) == 0) {
              std::cout << command << " is " << fullPath << std::endl;
              found = true;
              break;
            }
          }
          if (end == std::string::npos) {
            break;
          }
          start = end + 1;
          end = pathStr.find(':', start);
        }
        if (!found) {
          std::cout << command << ": not found" << std::endl;
         }
        
      }
   }
   else{
     std::cout<<input<<": command not found"<< std::endl;
   }
  
  }
}
