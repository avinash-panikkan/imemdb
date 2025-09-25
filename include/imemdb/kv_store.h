#ifndef IMEMDB_KV_STORE_H
#define IMEMDB_KV_STORE_H

#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

namespace imemdb {

class KeyValueStore
{
public:
    // Basic KV operations
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);

    // Persistence API
    bool save_to_file(const std::string& filename) const;
    bool load_from_file(const std::string& filename);

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::string> m_store;
};

} // namespace imemdb

#endif // IMEMDB_KV_STORE_H