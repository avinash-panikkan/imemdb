#include "include/imemdb/storage_engine.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

class StorageEngineTest : public ::testing::Test {
protected:
    std::string test_dir;
    std::string wal_path;
    std::string snapshot_path;

    void SetUp() override {
        test_dir = "testdata";
        wal_path = test_dir + "/wal.log";
        snapshot_path = test_dir + "/snapshot.rdb";
        fs::remove_all(test_dir);
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        fs::remove_all(test_dir);
    }

    static size_t count_lines(const std::string& path) {
        std::ifstream ifs(path);
        return std::count(std::istreambuf_iterator<char>(ifs),
                          std::istreambuf_iterator<char>(), '\n');
    }
};

// ---------------------------------------------------------------------------
// 1. Basic persistence cycle
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, BasicPutGetPersistence)
{
    {
        StorageEngine engine(wal_path, snapshot_path);
        engine.startup_recover();

        EXPECT_TRUE(engine.put("a", "1"));
        EXPECT_TRUE(engine.put("b", "2"));
        EXPECT_TRUE(engine.put("c", "3"));
        engine.flush_wal();

        EXPECT_TRUE(fs::exists(wal_path));
        EXPECT_GT(count_lines(wal_path), 0);

        EXPECT_TRUE(engine.snapshot_sync());
        engine.shutdown(true);
    }

    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();
    EXPECT_EQ(engine2.get("a").value_or(""), "1");
    EXPECT_EQ(engine2.get("b").value_or(""), "2");
    EXPECT_EQ(engine2.get("c").value_or(""), "3");
    engine2.shutdown(false);
}

// ---------------------------------------------------------------------------
// 2. Remove + overwrite correctness
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, RemoveAndOverwrite)
{
    StorageEngine engine(wal_path, snapshot_path);
    engine.startup_recover();

    EXPECT_TRUE(engine.put("temp", "100"));
    EXPECT_TRUE(engine.put("temp", "200"));
    EXPECT_TRUE(engine.remove("temp"));
    EXPECT_FALSE(engine.get("temp").has_value());

    engine.flush_wal();
    EXPECT_TRUE(engine.snapshot_sync());
    engine.shutdown(true);

    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();
    EXPECT_FALSE(engine2.get("temp").has_value());
    engine2.shutdown(false);
}

// ---------------------------------------------------------------------------
// 3. Concurrent writes + safe shutdown
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, ConcurrentWritesAndShutdown)
{
    StorageEngine engine(wal_path, snapshot_path);
    engine.startup_recover();

    std::atomic<bool> stop{false};
    std::thread writer([&]() {
        int i = 0;
        while (!stop) {
            engine.put("key_" + std::to_string(i), std::to_string(i));
            ++i;
        }
    });

    std::this_thread::sleep_for(50ms);
    stop = true;
    writer.join();

    engine.shutdown(true);

    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();

    bool foundOne = false;
    for (int i = 0; i < 500; ++i) {
        if (engine2.get("key_" + std::to_string(i)).has_value()) {
            foundOne = true;
            break;
        }
    }
    EXPECT_TRUE(foundOne);
    engine2.shutdown(false);
}

// ---------------------------------------------------------------------------
// 4. Crash-recovery using WAL only (simulate power loss before snapshot)
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, CrashRecoveryFromWALOnly)
{
    {
        StorageEngine engine(wal_path, snapshot_path);
        engine.startup_recover();
        EXPECT_TRUE(engine.put("x", "42"));
        EXPECT_TRUE(engine.put("y", "43"));
        // intentionally skip snapshot and shutdown abruptly
    } // engine destroyed, destructor not calling shutdown -> simulate crash

    // WAL should contain entries; snapshot absent
    EXPECT_TRUE(fs::exists(wal_path));
    EXPECT_FALSE(fs::exists(snapshot_path));

    // Recreate engine; must replay WAL
    StorageEngine recovered(wal_path, snapshot_path);
    recovered.startup_recover();

    EXPECT_EQ(recovered.get("x").value_or(""), "42");
    EXPECT_EQ(recovered.get("y").value_or(""), "43");

    recovered.shutdown(true);
}

// ---------------------------------------------------------------------------
// 5. Snapshot rotation test (WAL should shrink/rotate)
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, SnapshotRotationAndRecovery)
{
    StorageEngine engine(wal_path, snapshot_path);
    engine.startup_recover();

    // generate multiple writes to enlarge WAL
    for (int i = 0; i < 100; ++i)
        engine.put("k" + std::to_string(i), "v" + std::to_string(i));
    engine.flush_wal();
    size_t before = count_lines(wal_path);

    // trigger snapshot + rotation
    EXPECT_TRUE(engine.snapshot_sync());
    engine.wal_manager().rotate(engine.wal_manager().next_sequence() - 1);
    engine.shutdown(true);

    // WAL file should now be smaller (new file)
    size_t after = count_lines(wal_path);
    EXPECT_LT(after, before);

    // restart, ensure data still loads
    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();
    EXPECT_EQ(engine2.get("k10").value_or(""), "v10");
    engine2.shutdown(false);
}

// ---------------------------------------------------------------------------
// 6. Reject new operations after shutdown started
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, RejectWritesAfterShutdown)
{
    StorageEngine engine(wal_path, snapshot_path);
    engine.startup_recover();

    EXPECT_TRUE(engine.put("foo", "bar"));

    // start shutdown asynchronously
    std::thread shutdown_thread([&]() {
        engine.shutdown(true);
    });

    // wait briefly, then attempt to write
    std::this_thread::sleep_for(10ms);
    bool ok = engine.put("newkey", "value");
    EXPECT_FALSE(ok); // must be rejected

    shutdown_thread.join();

    // restart and ensure only "foo" persisted
    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();
    EXPECT_EQ(engine2.get("foo").value_or(""), "bar");
    EXPECT_FALSE(engine2.get("newkey").has_value());
    engine2.shutdown(false);
}

// ---------------------------------------------------------------------------
// 7. Thread-safety stress: concurrent readers + writers
// ---------------------------------------------------------------------------
TEST_F(StorageEngineTest, ThreadSafetyStress)
{
    StorageEngine engine(wal_path, snapshot_path);
    engine.startup_recover();

    std::atomic<bool> stop{false};
    std::thread writer([&]() {
        int i = 0;
        while (!stop) {
            engine.put("A" + std::to_string(i), "V" + std::to_string(i));
            ++i;
        }
    });

    std::thread reader([&]() {
        while (!stop) {
            auto v = engine.get("A10");
            (void)v;
        }
    });

    std::this_thread::sleep_for(100ms);
    stop = true;
    writer.join();
    reader.join();

    engine.shutdown(true);

    StorageEngine engine2(wal_path, snapshot_path);
    engine2.startup_recover();
    EXPECT_TRUE(engine2.get("A10").has_value());
    engine2.shutdown(false);
}
