#ifndef IMEMDB_COMMAND__H
#define IMEMDB_COMMAND__H

#include <string>
#include <vector>

struct Command {
    std::string strCmdName_;
    std::vector<std::string> args_;

    Command() = default;
    Command(std::string strCmdName, std::vector<std::string> args)
        : strCmdName_(std::move(strCmdName)), args_(std::move(args)) {};
};

#endif // IMEMDB_COMMAND__H