#ifndef IMEMDB_WAL_MANAGER_H
#define IMEMDB_WAL_MANAGER_H

#include <string>
#include <functional>
#include <mutex>
#include <atomic>

class WALManager
{
public:
    explicit WALManager(const std::string& filename);

    void append(uint64_t seq, const std::string& op, const std::string& key, const std::string& value = "");
    bool replay(uint64_t from_seq, std::function<void(const std::string&, const std::string&, const std::string&)> apply_func);

    uint64_t next_sequence() const { return next_seq_; }

private:
    std::string filename_;
    mutable std::mutex mutex_;
    std::atomic<uint64_t> next_seq_{1};
};

#endif // IMEMDB_WAL_MANAGER_H