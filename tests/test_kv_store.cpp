#include "imemdb.h"
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace imemdb;

int main() {
    {
        std::cout << "Running basic KV tests...\n";
        KeyValueStore store;

        store.put("name", "Alice");
        assert(store.get("name").has_value());
        assert(store.get("name").value() == "Alice");

        assert(store.remove("name") == true);
        assert(!store.get("name").has_value());
    }

    {
        std::cout << "Running persistence tests...\n";
        KeyValueStore store;
        store.put("user", "Bob");
        store.put("lang", "C++");

        const std::string filename = "testdata.txt";

        // Save
        assert(store.save_to_file(filename) == true);

        // Load into new instance
        KeyValueStore store2;
        assert(store2.load_from_file(filename) == true);

        assert(store2.get("user").has_value());
        assert(store2.get("user").value() == "Bob");

        assert(store2.get("lang").has_value());
        assert(store2.get("lang").value() == "C++");

        std::filesystem::remove(filename);
    }

    std::cout << "All tests passed!\n";
    return 0;
}
