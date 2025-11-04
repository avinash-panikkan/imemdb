#include "imemdb/command_validator.h"

const std::unordered_map<std::string, int> CommandValidator::expectedArgs_ = {
    {"put", 2},
    {"get", 1},
    {"remove", 1},
    {"exit", 0}
};

bool CommandValidator::validate(const Command &cmd, std::string &error)
{
    auto iter = expectedArgs_.find(cmd.strCmdName_);
    if (iter == expectedArgs_.end())
    {
        error = "Unknown command: " + cmd.strCmdName_;
        return false;
    }

    if (cmd.args_.size() != iter->second)
    {
        error = "Command `" + cmd.strCmdName_ + "` expects " + std::to_string(iter->second) + " arguments";
        return false;
    }

    return true;
}