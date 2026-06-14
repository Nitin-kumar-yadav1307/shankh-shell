#include "parser.h"


std::vector<std::string> splitInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken = "";
    bool singleQuoteMode = false;
    bool doubleQuoteMode = false;


    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
           
        if(c == '\\' && !doubleQuoteMode && !singleQuoteMode){
            i++;
            currentToken += input[i];
        }
        else if (c == '\'' && !doubleQuoteMode) {
            singleQuoteMode = !singleQuoteMode;  // toggle quote mode, don't add ' to token
        }
        else if (c == '\"' && !singleQuoteMode) {
            doubleQuoteMode = !doubleQuoteMode;  // toggle quote mode, don't add ' to token
        }
        else if(doubleQuoteMode && c == '\\'){
            char next = input[i+1];
            if(next == '\"' || next == '\\'){
                i++;
                currentToken += input[i];

            }
            else{
                currentToken += '\\';
            }
        }
       else if (c == ' ' && !singleQuoteMode && !doubleQuoteMode) {
        if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken = "";
        }
    }
    else if (c == '&' && !singleQuoteMode && !doubleQuoteMode) {
        if (!currentToken.empty()) {
             tokens.push_back(currentToken);
             currentToken = "";
    }
    tokens.push_back("&");
}
        else {
            currentToken += c;             // add character to current token
        }
    }

    if (!currentToken.empty()) {           // save last token
        tokens.push_back(currentToken);
    }

    return tokens;
}

