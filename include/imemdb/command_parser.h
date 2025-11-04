#ifndef IMEMDB_COMMAND_PARSER_H
#define IMEMDB_COMMAND_PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

struct Command {
    std::string strCmdName_;
    std::vector<std::string> args_;

    Command() = default;
    Command(std::string strCmdName, std::vector<std::string> args)
        : strCmdName_(std::move(strCmdName)), args_(std::move(args)) {};
};

class CommandParser
{
public:
    static Command parse_line(const std::string& strLine)
    {
        std::istringstream iss(strLine);
        std::string strCmdName;
        iss >> strCmdName;

        if (strCmdName.empty())
        {
            return Command{};
        }
        
        std::transform(strCmdName.begin(), strCmdName.end(), strCmdName.begin(),
                        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        std::string arg;
        std::vector<std::string> args;
        while (iss >> arg)
        {
            args.push_back(std::move(arg));
        }

        return Command{std::move(strCmdName), std::move(args)};
    } 
};

#endif // IMEMDB_COMMAND_PARSER_H