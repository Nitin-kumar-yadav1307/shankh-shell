#include "redirection.h"

#include <fcntl.h>
#include <unistd.h>

void applyRedirections(
    const std::string& redirectFile,
    bool appendMode,
    const std::string& stderrRedirectFile,
    bool stderrAppendMode,
    RedirectionContext& ctx
)
{
    if(!redirectFile.empty())
    {
        int flags =
            O_WRONLY |
            O_CREAT |
            (appendMode ? O_APPEND : O_TRUNC);

        int fd = open(
            redirectFile.c_str(),
            flags,
            0644
        );

        if(fd != -1)
        {
            ctx.savedStdout = dup(STDOUT_FILENO);

            dup2(fd, STDOUT_FILENO);

            close(fd);
        }
    }

    if(!stderrRedirectFile.empty())
    {
        int flags =
            O_WRONLY |
            O_CREAT |
            (stderrAppendMode ? O_APPEND : O_TRUNC);

        int fd = open(
            stderrRedirectFile.c_str(),
            flags,
            0644
        );

        if(fd != -1)
        {
            ctx.savedStderr = dup(STDERR_FILENO);

            dup2(fd, STDERR_FILENO);

            close(fd);
        }
    }
}

void restoreRedirections(
    RedirectionContext& ctx
)
{
    if(ctx.savedStdout != -1)
    {
        dup2(
            ctx.savedStdout,
            STDOUT_FILENO
        );

        close(ctx.savedStdout);

        ctx.savedStdout = -1;
    }

    if(ctx.savedStderr != -1)
    {
        dup2(
            ctx.savedStderr,
            STDERR_FILENO
        );

        close(ctx.savedStderr);

        ctx.savedStderr = -1;
    }
}