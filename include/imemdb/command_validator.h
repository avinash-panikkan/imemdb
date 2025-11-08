#ifndef IMEMDB_COMMAND_VALIDATOR_H
#define IMEMDB_COMMAND_VALIDATOR_H

#include <string>
#include <unordered_map>
#include "command.h"

class CommandValidator
{
public:
    static bool validate(const Command& cmd, std::string& error);

private:
    static const std::unordered_map<std::string, int> expectedArgs_;
};

#endif // IMEMDB_COMMAND_VALIDATOR_H