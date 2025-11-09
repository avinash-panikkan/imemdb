#include "imemdb/wal_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

WALManager::WALManager(const std::string &filename)
    : filename_(filename)
{
    // Determine last sequence number from existing WAL
    std::ifstream ifs(filename);
    std::string line;
    uint64_t last_seq = 0;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        uint64_t seq;
        if (iss >> seq)
            last_seq = seq;
    }
    next_seq_ = last_seq + 1;
}

void WALManager::append(uint64_t seq, const std::string &op, const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::ofstream ofs(filename_, std::ios::app);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Unable to open WAL file " << filename_ << "\n";
        return;
    }

    ofs << seq << '\t' << op << '\t' << key;
    if (!value.empty())
        ofs << '\t' << value;
    ofs << '\n';
    ofs.flush();
}

bool WALManager::replay(uint64_t from_seq, std::function<void(const std::string &, const std::string &, const std::string &)> apply_func)
{
    std::ifstream ifs(filename_);
    if (!ifs.is_open())
        return false;

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        uint64_t seq;
        std::string op, key, value;

        iss >> seq >> op >> key;
        std::getline(iss, value);
        if (!value.empty() && value[0] == '\t')
            value.erase(0, 1);

        if (seq > from_seq)
            apply_func(op, key, value);

        next_seq_ = std::max(next_seq_.load(), seq + 1);
    }

    return true;
}
