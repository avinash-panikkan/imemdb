#include "imemdb/kv_store.h"
#include <fstream>
#include <sstream>

namespace imemdb {

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

bool KeyValueStore::save_to_file(const std::string &filename) const
{
    std::unique_lock lock(m_mutex);

    std::ofstream ofs(filename, std::ios::trunc);
    if (!ofs.is_open())
        return false;
    
    for (const auto& [key, value] : m_store) {
        ofs << key << '\t' << value << '\n';
    }

    return true;
}
bool KeyValueStore::load_from_file(const std::string &filename)
{
    std::unique_lock lock(m_mutex);

    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return false;

    m_store.clear();
    std::string line;

    while (std::getline(ifs, line))
    {
        std::istringstream iss(line);
        std::string key, value;

        if (std::getline(iss, key, '\t') && std::getline(iss, value)) {
            m_store[key] = value;
        }
    }

    return true;
}
} // namespace imemdb