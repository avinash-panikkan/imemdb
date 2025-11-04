#include <iostream>
#include "imemdb.h"

int main()
{
    CommandDispatcher dispatcher;

    // Register command handlers
    // dispatcher.register_command("set")
    std::cout << "imemdb running!" << std::endl;
    while(true)
    {
        std::string strInput;
        std::getline(std::cin, strInput);

        Command cmd = CommandParser::parse_line(strInput);
        std::string error;
        if (CommandValidator::validate(cmd, error))
        {
            if (cmd.strCmdName_ == "exit")
            {
                std::cout << "Exiting application!" << std::endl;
                break;
            }

            std::cout << "Command: " << cmd.strCmdName_ << "\nArguments:";
            for (const auto& arg : cmd.args_) {
                std::cout << " " << arg;
            }
            std::cout << "\n";
        }
        else std::cout << error << "\n";
    }

    return 0;
}