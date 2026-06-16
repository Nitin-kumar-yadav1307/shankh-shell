#include "variable_manager.h"
#include <iostream>

std::map<std::string, std::string> shellVars;

bool isValidIdentifier(const std::string& name){
    if(name.empty()) return false;
    
    // first char must be letter or underscore
    if(!isalpha(name[0]) && name[0] != '_') return false;
    
    // rest must be letter, digit, or underscore
    for(size_t i = 1; i < name.size(); i++){
        if(!isalnum(name[i]) && name[i] != '_') return false;
    }
    
    return true;
}

std::string expandVariables(const std::string& token){
    std::string result;
    size_t i = 0;

    while(i < token.size()){

        // ${VAR}
        if(token[i] == '$' &&
           i + 1 < token.size() &&
           token[i + 1] == '{'){

            i += 2;   // skip "${"

            std::string name;

            while(i < token.size() && token[i] != '}'){
                name += token[i];
                i++;
            }

            // skip '}'
            if(i < token.size() && token[i] == '}')
                i++;

            if(shellVars.count(name))
                result += shellVars[name];
        }

        // $VAR
        else if(token[i] == '$'){
            i++;   // skip $

            std::string name;

            while(i < token.size() &&
                  (isalnum(token[i]) || token[i] == '_')){
                name += token[i];
                i++;
            }

            if(shellVars.count(name))
                result += shellVars[name];
        }

        else{
            result += token[i];
            i++;
        }
    }

    return result;
}

void builtinDeclare(const std::vector<std::string>& tokens){
    if(tokens.size() >= 2 && tokens[1] == "-p"){
        // print variable
        if(tokens.size() >= 3){
            std::string varName = tokens[2];
            if(shellVars.count(varName) > 0){
                std::cout << "declare -- " << varName
                          << "=\"" << shellVars[varName] 
                          << "\"\n";
            } else {
                std::cout << "declare: " << varName 
                          << ": not found\n";
            }
        }
    }
    else if(tokens.size() >= 2 && tokens[1].find('=') != std::string::npos){
        // store variable
        size_t eqPos = tokens[1].find('=');
        std::string name  = tokens[1].substr(0, eqPos);
        std::string value = tokens[1].substr(eqPos + 1);
        // validate name first
    if(!isValidIdentifier(name)){
        std::cout << "declare: `" << tokens[1] 
                  << "': not a valid identifier\n";
    } else {
        shellVars[name] = value;  // only store if valid
    }
    }
}