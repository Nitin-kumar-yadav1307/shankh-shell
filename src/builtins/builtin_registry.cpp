#include "builtin_registry.h"

#include "echo.h"
#include "pwd.h"
#include "cd.h"
#include "type.h"
#include "history.h"
#include "alias.h"
#include "export.h"
#include "unalias.h"
#include "source.h"
#include "fg.h"
#include "bg.h"
#include "jobs.h"
#include "kill.h" 
#include "env.h"
#include "unset.h"


#include "../variables/variable_manager.h"
#include "../jobs/job_manager.h"
#include "../completion/completion_builtin.h"

bool isBuiltin(const std::string& cmd)
{
    return cmd=="echo" ||
           cmd=="pwd"  ||
           cmd=="type" ||
           cmd=="cd"   ||
           cmd=="exit" ||
           cmd=="jobs" ||
           cmd=="kill" || 
           cmd=="env" ||
           cmd=="unset" ||
           cmd=="history" ||
           cmd == "complete" ||
           cmd == "alias" ||
           cmd == "export" ||
           cmd == "unalias" ||
           cmd == "source" ||
           cmd=="fg"||
           cmd == "bg"||
           cmd=="declare";
}

void runBuiltin(std::vector<std::string>& toks)
{
    if(toks.empty())
        return;

    if(toks[0] == "echo")
    {
        builtinEcho(toks);
    }
    else if(toks[0] == "pwd")
    {
        builtinPwd();
    }
    else if(toks[0] == "cd")
    {
        std::string path = "~";

        if(toks.size() > 1)
            path = toks[1];

        builtinCd(path);
    }
    else if(toks[0] == "type")
    {
        builtinType(toks);
    }
    else if(toks[0] == "history")
    {
        builtinHistory(toks);
    }
    else if(toks[0] == "declare")
    {
        builtinDeclare(toks);
    }
    else if(toks[0] == "complete")
    {
        builtinComplete(toks);
    }
    else if(toks[0] == "jobs")
    {
        printJobs();
    }
    else if(toks[0] == "kill") 
    {
        builtinKill(toks);
    }
    else if(toks[0] == "env")
    {
        builtinEnv(toks);
    }
    else if(toks[0] == "unset")
    {
        builtinUnset(toks);
    }
    else if(toks[0] == "alias")
    {
        builtinAlias(toks);
    }
    else if(toks[0] == "export")
    {
        builtinExport(toks);
    }
    else if(toks[0] == "unalias")
    {
        builtinUnalias(toks);
    }
    else if(toks[0] == "source")
    {
        builtinSource(toks);
    }
    else if(toks[0] == "fg")
    {
        builtinFg(toks);
    }
    else if(toks[0] == "bg")
    {
        builtinBg(toks);
    }
}