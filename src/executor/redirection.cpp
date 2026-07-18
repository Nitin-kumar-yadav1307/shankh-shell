#include "redirection.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

void applyRedirections(
    const std::string& redirectFile,
    bool appendMode,
    const std::string& stderrRedirectFile,
    bool stderrAppendMode,
    bool duplicateStderr, // <-- Hamara naya parameter
    RedirectionContext& ctx
)
{
    
    // 1. Normal Output Redirection (>)
    if (!redirectFile.empty()) {
        int flags = O_WRONLY | O_CREAT | (appendMode ? O_APPEND : O_TRUNC);
        int fd = open(redirectFile.c_str(), flags, 0644);
        if (fd != -1) {
            ctx.savedStdout = dup(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO); // FD 1 ab file ko point kar raha hai
            close(fd);
        } else {
            perror("Failed to open redirect file");
        }
    }

    // 2. Normal Error Redirection (2>)
    if (!stderrRedirectFile.empty()) {
        int flags = O_WRONLY | O_CREAT | (stderrAppendMode ? O_APPEND : O_TRUNC);
        int fd = open(stderrRedirectFile.c_str(), flags, 0644);
        if (fd != -1) {
            ctx.savedStderr = dup(STDERR_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
    }

    // 3. THE MAGIC WIRING (2>&1)
    // DHYAN DEIN: Yeh hamesha STDOUT ke redirect hone ke BAAD aana chahiye!
    if (duplicateStderr) {
        ctx.savedStderr = dup(STDERR_FILENO);
        
        // Port 2 (stderr) ko uthao, aur usko wahan point kar do jahan Port 1 (stdout) jaa raha hai!
        dup2(STDOUT_FILENO, STDERR_FILENO); 
    }
}