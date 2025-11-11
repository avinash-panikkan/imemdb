#include "imemdb/kv_store.h"

void KeyValueStore::put(const std::string &key, const std::string &value)
{
    std::unique_lock lock(mutex_);
    store_[key] = value;
}

std::optional<std::string> KeyValueStore::get(const std::string &key) const
{
    std::shared_lock lock(mutex_);
    auto iter = store_.find(key);
    if (iter != store_.end())
        return iter->second;

    return std::nullopt;
}

bool KeyValueStore::remove(const std::string &key)
{
    std::unique_lock lock(mutex_);
    return store_.erase(key) > 0;
}

const std::unordered_map<std::string, std::string> &KeyValueStore::data() const
{
    std::shared_lock lock(mutex_);
    return store_;
}

void KeyValueStore::load_data(std::unordered_map<std::string, std::string> &&data)
{
    std::unique_lock lock(mutex_);
    store_ = std::move(data);
}