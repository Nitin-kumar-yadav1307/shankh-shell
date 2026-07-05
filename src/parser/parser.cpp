#include "parser.h"

std::vector<std::string> splitInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken = "";
    bool singleQuoteMode = false;
    bool doubleQuoteMode = false;

    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
            
        if (c == '\\' && !doubleQuoteMode && !singleQuoteMode) {
            i++;
            if (i < input.length()) {
                currentToken += input[i];
            }
        }
        else if (c == '\'' && !doubleQuoteMode) {
            singleQuoteMode = !singleQuoteMode;  // toggle single quote mode
        }
        else if (c == '\"' && !singleQuoteMode) {
            doubleQuoteMode = !doubleQuoteMode;  // toggle double quote mode
        }
        else if (doubleQuoteMode && c == '\\') {
            if (i + 1 < input.length()) {
                char next = input[i+1];
                if (next == '\"' || next == '\\') {
                    i++;
                    currentToken += input[i];
                } else {
                    currentToken += '\\';
                }
            }
        }
        else if (c == ' ' && !singleQuoteMode && !doubleQuoteMode) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken = "";
            }
        }
        // NEW LOGIC: Handle control operators (&, &&, |, ||, ;)
        else if ((c == '&' || c == '|' || c == ';') && !singleQuoteMode && !doubleQuoteMode) {
            // Push any pending token (like "ls" before encountering "&&")
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken = "";
            }

            if (c == '&') {
                // Peek ahead to check if it's "&&"
                if (i + 1 < input.length() && input[i+1] == '&') {
                    tokens.push_back("&&");
                    i++; // Skip the second '&'
                } else {
                    tokens.push_back("&"); // Just a background operator
                }
            } 
            else if (c == '|') {
                // Peek ahead to check if it's "||"
                if (i + 1 < input.length() && input[i+1] == '|') {
                    tokens.push_back("||");
                    i++; // Skip the second '|'
                } else {
                    tokens.push_back("|"); // Just a standard pipe
                }
            } 
            else if (c == ';') {
                tokens.push_back(";");
            }
        }
        else {
            currentToken += c; // add character to current token
        }
    }

    if (!currentToken.empty()) { // save last token
        tokens.push_back(currentToken);
    }

    return tokens;
}

std::vector<CommandBlock> parseTokens(const std::vector<std::string>& tokens) {
    std::vector<CommandBlock> blocks;
    CommandBlock currentBlock;

    for (const std::string& token : tokens) {
        if (token == "&&") {
            currentBlock.op = CommandOperator::AND;
            blocks.push_back(currentBlock);
            currentBlock = CommandBlock(); // Reset for the next command
        } else if (token == "||") {
            currentBlock.op = CommandOperator::OR;
            blocks.push_back(currentBlock);
            currentBlock = CommandBlock();
        } else if (token == ";") {
            currentBlock.op = CommandOperator::SEQUENCE;
            blocks.push_back(currentBlock);
            currentBlock = CommandBlock();
        } else if (token == "|") {
            currentBlock.op = CommandOperator::PIPE;
            blocks.push_back(currentBlock);
            currentBlock = CommandBlock();
        } else if (token == "&") {
            currentBlock.op = CommandOperator::BACKGROUND;
            blocks.push_back(currentBlock);
            currentBlock = CommandBlock();
        } else {
            // It's a normal argument, add it to the current block
            currentBlock.args.push_back(token);
        }
    }

    // Push the final block if it contains any arguments 
    // (its operator will default to CommandOperator::NONE)
    if (!currentBlock.args.empty()) {
        blocks.push_back(currentBlock);
    }

    return blocks;
}