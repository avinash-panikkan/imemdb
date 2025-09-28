#include "imemdb/kv_store.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace imemdb {

KeyValueStore::KeyValueStore(const std::string &wal_filename)
    : m_wal_filename(wal_filename)
{
    if (!wal_filename.empty())
    {
        if (!load_from_file(wal_filename))
        {
            std::cerr << "Warning : Could not load WAL file\n";
        }
    }
}

void KeyValueStore::put(const std::string &key, const std::string &value)
{
    std::unique_lock lock(m_mutex);
    m_store[key] = value;
    append_wal("PUT\t" + key + "\t" + value);
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
    bool existed = m_store.erase(key) > 0;
    if (existed)
        append_wal("REMOVE\t" + key);

    return existed;
}

bool KeyValueStore::save_to_file(const std::string &filename) const
{
    std::unique_lock lock(m_mutex);

    std::ofstream ofs(filename, std::ios::trunc);
    if (!ofs.is_open())
        return false;
    
    for (const auto& [key, value] : m_store) 
    {
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
        if (line.empty())
            continue;
        
        std::istringstream iss(line);
        std::string first, key, value;

        if (!std::getline(iss, first, '\t'))
            continue; 

        if (first == "PUT")
        {
            if (std::getline(iss, key, '\t') && (std::getline(iss, value)))
                m_store[key] = value;
        }
        else if (first == "REMOVE")
        {
            if (std::getline(iss, key))
                m_store.erase(key);
        }
        else
        {
            key = first;
            if (std::getline(iss, value))
                m_store[key] = value;
        }
    }

    return true;
}

void KeyValueStore::append_wal(const std::string &entry)
{
    if (m_wal_filename.empty())
        return;
    
    std::ofstream ofs(m_wal_filename, std::ios::app);
    if (!ofs.is_open())
    {
        std::cerr << "Error : Cannot open WAL file\n";
        return;
    }

    ofs << entry << "\n"; 
}

} // namespace imemdb