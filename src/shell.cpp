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

void executeCommand(const std::string& input)
{
    if(input.empty())
        return;

    std::string redirectFile = "";
    int pipeIndex = -1;
    int stderrIndexToken = -1;
    std::string stderrRedirectFile = "";

    bool appendMode = false;
    bool stderrAppendMode = false;

    std::vector<std::string> tokens = splitInput(input);

    expandAlias(tokens);

    // expand variables
    for(auto& token : tokens)
    {
        if(token != "|" &&
           token != ">" &&
           token != ">>" &&
           token != "2>" &&
           token != "2>>" &&
           token != "&" &&
           token != "1>" &&
           token != "1>>")
        {
            token = expandVariables(token);
        }
    }

    if(tokens.empty())
        return;

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
        {
            return;
        }

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
        executePipeline(tokens);
        return;
    }

    if(command == "exit"){
        char* histFile = getenv("HISTFILE");

        if(histFile){
            write_history(histFile);
        }

        exit(0);
    }

    if(isBuiltin(command)){
        runBuiltin(tokens);
    }
    else{
        executeExternal(
            tokens,
            background,
            redirectFile,
            appendMode,
            stderrRedirectFile,
            stderrAppendMode
        );
    }
}