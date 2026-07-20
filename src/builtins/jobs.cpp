#include "jobs.h"
#include "../jobs/job_manager.h" // Job manager ko link kar rahe hain
#include <iostream>

void builtinJobs(const std::vector<std::string>& tokens) {
      
    // Note: Humein check karna hoga ki tumhare job_manager.h mein printJobs() ka actual naam kya hai.
    // Standard naam printJobs() ya listJobs() hota hai. Main printJobs() assume kar raha hu.
    
    printJobs(); 
}