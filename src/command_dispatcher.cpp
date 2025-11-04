#include "imemdb/command_dispatcher.h"

void CommandDispatcher::register_command(const std::string& cmdName, CommandHandler handler)
{
    handlers_[cmdName] = std::move(handler);
}

bool CommandDispatcher::dispatch(const Command& cmd, std::string& error)
{
    auto iter = handlers_.find(cmd.strCmdName_);
    if (iter == handlers_.end())
    {
        error = "Unknown command" + cmd.strCmdName_;
        return false;
    }

    try
    {
        iter->second(cmd.args_);
    }
    catch(const std::exception& e)
    {
        error = "Error executing command: " + std::string(e.what());
        return false;
    }

    return true;
}