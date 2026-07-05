#include "shell.h"


#include "parser/parser.h"
#include "aliases/alias_manager.h"
#include "variables/variable_manager.h"
#include "builtins/builtin_registry.h"
#include "executor/pipeline_executor.h"
#include "executor/external_executor.h"

#include <readline/history.h>
#include <string>
#include <vector>
#include <iostream>
#include <csignal>

void handleSigInt(int sig) {
    // When Ctrl+C is pressed, we want to move to a new line and show the prompt.
    // Note: If you have a custom prompt string, replace "shankh> " with yours.
    std::cout << "\n$ ";; 
    
    // Force the output to display immediately
    std::cout.flush(); 
}

void setupSignalHandlers() {
    // Catch Ctrl+C and route it to our custom handler
    signal(SIGINT, handleSigInt);
    
    // Ignore Ctrl+\ (Quit) and Ctrl+Z (Suspend) so the shell itself doesn't stop
    // Note: Child processes will still receive these signals, which is correct!
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

// Forward declaration for the new helper function
int executeSingleCommand(std::vector<std::string>& tokens);

void executeCommand(const std::string& input)
{
    if(input.empty())
        return;

    std::vector<std::string> tokens = splitInput(input);
    expandAlias(tokens);

    // Expand variables, but skip our new control flow operators
    for(auto& token : tokens)
    {
        if(token != "|" && token != ">" && token != ">>" &&
           token != "2>" && token != "2>>" && token != "&" &&
           token != "1>" && token != "1>>" &&
           token != "&&" && token != "||" && token != ";")
        {
            token = expandVariables(token);
        }
    }

    if(tokens.empty())
        return;

    // --- NEW LOGIC: Control Flow for &&, ||, and ; ---
    std::vector<std::string> currentCommand;
    bool skipNext = false;
    int lastStatus = 0; // 0 indicates success in POSIX systems

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i] == "&&" || tokens[i] == "||" || tokens[i] == ";")
        {
            // Execute the accumulated command block before processing the operator
            if (!currentCommand.empty())
            {
                if (!skipNext) {
                    lastStatus = executeSingleCommand(currentCommand);
                }
                currentCommand.clear();
            }

            // Determine if the *next* command block should be skipped
            if (tokens[i] == "&&") {
                skipNext = (lastStatus != 0); // Skip if the previous command failed
            } else if (tokens[i] == "||") {
                skipNext = (lastStatus == 0); // Skip if the previous command succeeded
            } else if (tokens[i] == ";") {
                skipNext = false; // Semicolons always execute the next block
            }
        }
        else
        {
            // Add normal arguments, pipes, and redirects to the current block
            currentCommand.push_back(tokens[i]);
        }
    }

    // Execute the final pending command in the chain
    if (!currentCommand.empty() && !skipNext)
    {
        executeSingleCommand(currentCommand);
    }
}

// --- NEW HELPER: Contains your exact original execution logic ---
int executeSingleCommand(std::vector<std::string>& tokens)
{
    std::string redirectFile = "";
    int pipeIndex = -1;
    int stderrIndexToken = -1;
    std::string stderrRedirectFile = "";

    bool appendMode = false;
    bool stderrAppendMode = false;
    bool background = false;

    if(!tokens.empty() && tokens.back() == "&")
    {
        background = true;
        tokens.pop_back();
    }

    while(!tokens.empty() && tokens.back().empty())
    {
        tokens.pop_back();
    }

    if(tokens.empty())
        return 0;

    std::string command = tokens[0];

    for(int i = 0; i < tokens.size(); i++)
    {
        if(tokens[i] == "|")
        {
            pipeIndex = i;
            break;
        }
        if(tokens[i] == "2>>")
        {
            stderrAppendMode = true;
            stderrIndexToken = i;
            stderrRedirectFile = tokens[i + 1];
            break;
        }
        if(tokens[i] == "2>")
        {
            stderrAppendMode = false;
            stderrIndexToken = i;
            stderrRedirectFile = tokens[i + 1];
            break;
        }
        if(tokens[i] == ">>" || tokens[i] == "1>>")
        {
            appendMode = true;
            redirectFile = tokens[i + 1];
            break;
        }
        if(tokens[i] == ">" || tokens[i] == "1>")
        {
            appendMode = false;
            redirectFile = tokens[i + 1];
            break;
        }
    }

    if(pipeIndex != -1){
        // TODO: Update executePipeline to return an int status instead of void
        executePipeline(tokens);
        return 0; 
    }

    if(command == "exit"){
        char* histFile = getenv("HISTFILE");
        if(histFile){
            write_history(histFile);
        }
        exit(0);
    }

    if(isBuiltin(command)){
        // TODO: Update runBuiltin to return an int status instead of void
        runBuiltin(tokens);
        return 0;
    }
    else{
        // Return the actual exit status up the chain!
        return executeExternal(
            tokens,
            background,
            redirectFile,
            appendMode,
            stderrRedirectFile,
            stderrAppendMode
        );
    }
}