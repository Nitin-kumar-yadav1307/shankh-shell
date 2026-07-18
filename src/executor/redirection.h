#pragma once

#include <string>

struct RedirectionContext
{
    int savedStdout = -1;
    int savedStderr = -1;
};

void applyRedirections(
    const std::string& redirectFile,
    bool appendMode,
    const std::string& stderrRedirectFile,
    bool stderrAppendMode,
    bool duplicateStderr,
    RedirectionContext& ctx
);

void restoreRedirections(
    RedirectionContext& ctx
);