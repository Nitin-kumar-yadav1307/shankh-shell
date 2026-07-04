#pragma once

#include <string>
#include <csignal>

void executeCommand(const std::string& input);
void setupSignalHandlers();