#include <iostream>
#include "imemdb.h"

void register_commands(CommandDispatcher& dispatcher, KeyValueStore& kv_store, bool& shouldExit)
{
    dispatcher.register_command("help", [&](const std::vector<std::string>&) {
        std::cout << "Commands:\n"
                << "put <key> <value>  - store a value\n"
                << "get <key>          - retrieve a value\n"
                << "remove <key>       - delete a key\n"
                << "exit               - quit\n";
    });

    dispatcher.register_command("put", [&](const std::vector<std::string>& args) {
        kv_store.put(args[0], args[1]);
        std::cout << "OK\n";
    });

    dispatcher.register_command("get", [&](const std::vector<std::string>& args) {
        std::optional<std::string> val = kv_store.get(args[0]);
        if (val.has_value())
            std::cout << val.value() << "\n";
        else
            std::cout << "(nil)\n";
    });

    dispatcher.register_command("remove", [&](const std::vector<std::string>& args) {
        if (kv_store.remove(args[0]))
        {
            std::cout << "REMOVED\n";
        }
        else
        {
            std::cout << "NO ITEM\n";
        }
    });

    dispatcher.register_command("exit", [&](const std::vector<std::string>&) {
        std::cout << "Exiting...\n";
        shouldExit = true;
    });
}

int main()
{
    KeyValueStore kv_store;
    CommandDispatcher dispatcher;
    bool shouldExit = false;
    register_commands(dispatcher, kv_store, shouldExit);

    std::cout << "imemdb running!" << std::endl;
    while(!shouldExit)
    {
        std::string strInput;
        std::getline(std::cin, strInput);

        Command cmd = CommandParser::parse_line(strInput);
        if (cmd.strCmdName_.empty())
        {
            continue;
        }

        std::string error;
        if (CommandValidator::validate(cmd, error))
        {
            if (!dispatcher.dispatch(cmd, error))
            {
                std::cout << error << "\n";
            }
        }
        else 
        {
            std::cout << error << "\n";
        }
    }

    return 0;
}