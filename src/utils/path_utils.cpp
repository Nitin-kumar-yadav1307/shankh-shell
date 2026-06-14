#include "path_utils.h"

#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
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
