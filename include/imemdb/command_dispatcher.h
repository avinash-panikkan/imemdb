#ifndef IMEMDB_COMMAND_DISPATCHER_H
#define IMEMDB_COMMAND_DISPATCHER_H

#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include "imemdb/command_parser.h"

class CommandDispatcher
{
public:
    using CommandHandler = std::function<void(const std::vector<std::string>&)>;

    void register_command(const std::string& cmdName, CommandHandler handler);
    bool dispatch(const Command& cmd, std::string& error);

private:
    std::unordered_map<std::string, CommandHandler> handlers_;
};

#endif //IMEMDB_COMMAND_DISPATCHER_H