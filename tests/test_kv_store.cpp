#include "imemdb/kv_store.h"
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace imemdb;

int main() {
    const std::string wal_file = "wal_test.txt";
    const std::string snapshot_file = "snapshot_test.txt";
    std::filesystem::remove(wal_file);
    std::filesystem::remove(snapshot_file);

    // ---- WAL TEST ----
    {
        KeyValueStore kv(wal_file);
        kv.put("x", "100");
        kv.put("y", "200");
        kv.remove("y");

        KeyValueStore kv2(wal_file); // load from WAL
        assert(kv2.get("x").value() == "100");
        assert(!kv2.get("y").has_value());
    }

    // ---- SNAPSHOT TEST ----
    {
        KeyValueStore kv;
        kv.put("a", "foo");
        kv.put("b", "bar");

        assert(kv.save_to_file(snapshot_file));

        KeyValueStore kv2;
        kv2.load_from_file(snapshot_file); // load from snapshot
        assert(kv2.get("a").value() == "foo");
        assert(kv2.get("b").value() == "bar");
    }

    std::filesystem::remove(wal_file);
    std::filesystem::remove(snapshot_file);

    std::cout << "WAL + Snapshot tests passed!\n";
    return 0;
}
