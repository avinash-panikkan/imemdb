#include "imemdb/snapshot_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

SnapshotManager::SnapshotManager(const std::string& directory)
    : directory_(directory)
{
    std::filesystem::create_directories(directory);
}

bool SnapshotManager::save(const std::unordered_map<std::string, std::string>& data, uint64_t seq)
{
    auto now = std::chrono::system_clock::now();
    auto current_time = std::chrono::system_clock::to_time_t(now);

    std::ostringstream fname;
    fname << directory_ << "/snapshot-" << std::put_time(std::localtime(&current_time), "%Y%m%d-%H%M%S") << ".rdb";
    std::string tmp = fname.str() + ".tmp";

    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs.is_open())
        return false;

    ofs << "# snapshot_seq=" << seq << "\n";
    for (auto& [key, value] : data)
        ofs << key << '\t' << value << '\n';

    ofs.flush();
    ofs.close();

    std::filesystem::rename(tmp, fname.str());

    // Update manifest
    std::ofstream manifest(directory_ + "/CURRENT", std::ios::trunc);
    manifest << std::filesystem::path(fname.str()).filename().string();

    return true;
}

std::string SnapshotManager::latest_snapshot_file() const
{
    std::ifstream manifest(directory_ + "/CURRENT");
    std::string fname;
    if (manifest.is_open() && std::getline(manifest, fname))
        return directory_ + "/" + fname;

    // fallback: pick latest timestamp file
    std::string latest;
    std::filesystem::file_time_type latest_time;
    for (auto& entry : std::filesystem::directory_iterator(directory_))
    {
        if (entry.path().extension() == ".rdb")
        {
            auto ftime = entry.last_write_time();
            if (latest.empty() || ftime > latest_time)
            {
                latest_time = ftime;
                latest = entry.path().string();
            }
        }
    }
    return latest;
}

bool SnapshotManager::load(std::unordered_map<std::string, std::string>& data, uint64_t& snapshot_seq)
{
    std::string file = latest_snapshot_file();
    if (file.empty())
        return false;

    std::ifstream ifs(file);
    if (!ifs.is_open())
        return false;

    data.clear();
    std::string line;
    snapshot_seq = 0;

    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        if (line.rfind("# snapshot_seq=", 0) == 0)
        {
            snapshot_seq = std::stoull(line.substr(16));
            continue;
        }

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '\t') && std::getline(iss, value))
            data[key] = value;
    }
    return true;
}
