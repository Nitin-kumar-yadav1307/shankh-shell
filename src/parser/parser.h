#pragma once

#include <string>
#include <vector>

// Define the relationships between commands
enum class CommandOperator {
    NONE,       // Just a standard command (e.g., the last command in a line)
    PIPE,       // |  (Pipe output to next command)
    AND,        // && (Run next if this succeeds)
    OR,         // || (Run next if this fails)
    SEQUENCE,   // ;  (Run next regardless)
    BACKGROUND  // &  (Run in background)
};

// Represents a single command and the operator that follows it
struct CommandBlock {
    std::vector<std::string> args; // e.g., ["ls", "-la"]
    CommandOperator op = CommandOperator::NONE; // What comes after? (e.g., AND)
};

// Your existing tokenizer
std::vector<std::string> splitInput(const std::string& input);

// NEW: Function to group raw tokens into structured command blocks
std::vector<CommandBlock> parseTokens(const std::vector<std::string>& tokens);