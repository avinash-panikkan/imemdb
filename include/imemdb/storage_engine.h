#ifndef IMEMDB_STORAGE_ENGINE_H
#define IMEMDB_STORAGE_ENGINE_H

#include <string>
#include <thread>
#include <atomic>
#include <deque>
#include <condition_variable>
#include <functional>
#include <optional>

#include "imemdb/kv_store.h"
#include "imemdb/wal_manager.h"
#include "imemdb/snapshot_manager.h"

class StorageEngine {
public:
    struct Options {
        std::string wal_filename;
        std::string snapshot_dir;
    };

    explicit StorageEngine(const Options& opts);
    ~StorageEngine();

    // Startup recovery: load snapshot then replay WAL
    bool startup_recover();

    // Basic API (thread-safe)
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);

    // Trigger snapshot (async). Returns immediately.
    void snapshot_async();

    // Synchronous snapshot (useful for shutdown)
    bool snapshot_sync();

    // Graceful shutdown: flush WAL writer and optionally snapshot
    void shutdown(bool take_final_snapshot = true);

    // For tests / direct control
    void flush_wal(); // block until WAL queue empty and writes flushed

private:
    // WAL work item
    struct WalEntry {
        uint64_t seq;
        std::string op;   // "PUT" or "REMOVE"
        std::string key;
        std::string value;
    };

    // Background threads
    void wal_writer_thread_main();
    void snapshot_thread_main();

    // enqueue wal entry (non-blocking)
    void enqueue_wal(const WalEntry& entry);

    // helpers
    bool replay_wal_after_snapshot(uint64_t snapshot_seq);

private:
    Options opts_;
    KeyValueStore kv_store_;
    WALManager wal_manager_;
    SnapshotManager snapshot_manager_;

    // WAL support
    std::thread wal_thread_;
    std::mutex wal_mutex_;
    std::condition_variable wal_cv_;
    std::deque<WalEntry> wal_queue_;
    std::atomic<bool> wal_running_{false};

    // Snapshot support
    std::thread snapshot_thread_;
    std::mutex snapshot_mutex_;
    std::condition_variable snapshot_cv_;
    std::atomic<bool> snapshot_requested_{false};
    std::atomic<bool> snapshot_running_{false};

    // Shutdown flag
    std::atomic<bool> shutdown_{false};
};

#endif // IMEMDB_STORAGE_ENGINE_H
