#ifndef IMEMDB_KV_STORE_H
#define IMEMDB_KV_STORE_H

#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

class KeyValueStore
{
public:
    KeyValueStore() = default;

    // Basic KV operations
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);

    // Snapshot helpers
    const std::unordered_map<std::string, std::string>& data() const;
    void load_data(std::unordered_map<std::string, std::string>&& data);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> store_;
};

#endif // IMEMDB_KV_STORE_H