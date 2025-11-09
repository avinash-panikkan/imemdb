#include "imemdb/kv_store.h"

void KeyValueStore::put(const std::string &key, const std::string &value)
{
    std::unique_lock lock(m_mutex);
    m_store[key] = value;
}

std::optional<std::string> KeyValueStore::get(const std::string &key) const
{
    std::shared_lock lock(m_mutex);
    auto iter = m_store.find(key);
    if (iter != m_store.end())
        return iter->second;

    return std::nullopt;
}

bool KeyValueStore::remove(const std::string &key)
{
    std::unique_lock lock(m_mutex);
    return m_store.erase(key) > 0;
}

void KeyValueStore::load_data(std::unordered_map<std::string, std::string> &&data)
{
    std::unique_lock lock(m_mutex);
    m_store = std::move(data);
}