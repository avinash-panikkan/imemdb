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
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::string> m_store;
};

} // namespace imemdb

#endif // IMEMDB_KV_STORE_H