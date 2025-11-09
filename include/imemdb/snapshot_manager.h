#ifndef IMEMDB_SNAPSHOT_MANAGER_H
#define IMEMDB_SNAPSHOT_MANAGER_H

#include <string>
#include <unordered_map>

class SnapshotManager
{
public:
    explicit SnapshotManager(const std::string& directory);

    bool save(const std::unordered_map<std::string, std::string>& data, uint64_t seq);
    bool load(std::unordered_map<std::string, std::string>& data, uint64_t& snapshot_seq);
    std::string latest_snapshot_file() const;

private:
    std::string directory_;
};

#endif // IMEMDB_SNAPSHOT_MANAGER_H
