#include "imemdb.h"
#include <cassert>
#include <iostream>

int main() {
    imemdb::KeyValueStore kv;

    // 1 Test put and get
    kv.put("name", "Author");
    kv.put("project", "imemdb");

    auto name = kv.get("name");
    assert(name.has_value());
    assert(name.value() == "Author");

    auto project = kv.get("project");
    assert(project.has_value());
    assert(project.value() == "imemdb");

    std::cout << "Put/Get test passed\n";

    // 2️ Test update
    kv.put("name", "UpdatedName");
    name = kv.get("name");
    assert(name.has_value());
    assert(name.value() == "UpdatedName");

    std::cout << "Update test passed\n";

    // 3️ Test remove
    bool removed = kv.remove("name");
    assert(removed);
    assert(!kv.get("name").has_value());

    removed = kv.remove("nonexistent");
    assert(!removed);

    std::cout << "Remove test passed\n";

    // 4️ Test multiple keys
    kv.put("key1", "val1");
    kv.put("key2", "val2");
    kv.put("key3", "val3");

    assert(kv.get("key1").value() == "val1");
    assert(kv.get("key2").value() == "val2");
    assert(kv.get("key3").value() == "val3");

    std::cout << "Multiple keys test passed\n";

    std::cout << "\nAll tests passed successfully!\n";
    return 0;
}
