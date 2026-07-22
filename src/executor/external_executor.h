#pragma once
#include <vector>
#include <string>

int executeExternal(
    const std::vector<std::string>& tokens,
    bool background,
    const std::string& redirectFile,
    bool appendMode,
    const std::string& stderrRedirectFile,
    bool stderrAppendMode,
    bool duplicateStderr );
    
void executePipeline(const std::vector<std::string>& tokens);