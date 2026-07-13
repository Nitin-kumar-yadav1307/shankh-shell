#include "path_utils.h"

#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <glob.h>
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

void expandGlobs(std::vector<std::string>& tokens) {
    std::vector<std::string> expandedTokens;

    for (const auto& token : tokens) {
        // Only attempt to glob if the token contains a wildcard
        if (token.find('*') != std::string::npos || token.find('?') != std::string::npos) {
            glob_t glob_result;
            
            // GLOB_NOCHECK: If no files match, return the original wildcard string (standard bash behavior)
            // GLOB_TILDE: Also expand '~' to the user's home directory
            int return_value = glob(token.c_str(), GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result);

            if (return_value == 0) {
                // Add all matching filenames to our new list
                for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
                    expandedTokens.push_back(std::string(glob_result.gl_pathv[i]));
                }
            } else {
                // Fallback: If glob fails entirely, keep the original token
                expandedTokens.push_back(token);
            }
            
            // Free the memory allocated by the glob function
            globfree(&glob_result);
        } else {
            // Not a wildcard, just pass it through
            expandedTokens.push_back(token);
        }
    }

    // Replace the original token list with our newly expanded list
    tokens = expandedTokens;
}