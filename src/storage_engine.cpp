#include "imemdb/storage_engine.h"
#include <iostream>
#include <chrono>
#include <thread>

StorageEngine::StorageEngine(const Options& opts)
    : opts_(opts),
    wal_manager_(opts.wal_filename),
    snapshot_manager_(opts.snapshot_dir)
{
    // Start WAL writer thread
    wal_running_ = true;
    wal_thread_ = std::thread(&StorageEngine::wal_writer_thread_main, this);

    // Start snapshot thread
    snapshot_running_ = true;
    snapshot_thread_ = std::thread(&StorageEngine::snapshot_thread_main, this);
}

StorageEngine::~StorageEngine()
{
    shutdown(true);

    if (wal_thread_.joinable())
        wal_thread_.join();

    if (snapshot_thread_.joinable())
        snapshot_thread_.join();
}

// Replay snapshot (if any) and WAL after the snapshot seq.
bool StorageEngine::startup_recover()
{
    uint64_t snapshot_seq = 0;
    std::unordered_map<std::string, std::string> data;

    bool loaded = snapshot_manager_.load(data, snapshot_seq);
    if (loaded) {
        std::cout << "Loaded snapshot with seq " << snapshot_seq << "\n";
        kv_store_.load_data(std::move(data));
    } else {
        std::cout << "No snapshot found (or failed to load).\n";
    }

    // Replay WAL entries with seq > snapshot_seq
    bool ok = replay_wal_after_snapshot(snapshot_seq);
    if (!ok) {
        std::cerr << "Warning: failed to replay WAL\n";
    }
    return ok;
}

bool StorageEngine::replay_wal_after_snapshot(uint64_t snapshot_seq)
{
    auto apply = [this](const std::string& op, const std::string& key, const std::string& value) {
        if (op == "PUT" || op == "put" || op == "Put")
        {
            kv_store_.put(key, value);
        }
        else if (op == "REMOVE" || op == "remove" || op == "Remove")
        {
            kv_store_.remove(key);
        }
        else
        {
            // unknown op - ignore or log
        }
    };
    return wal_manager_.replay(snapshot_seq, apply);
}

void StorageEngine::enqueue_wal(const WalEntry& entry)
{
    {
        std::lock_guard lock(wal_mutex_);
        wal_queue_.push_back(entry);
    }
    wal_cv_.notify_one();
}

void StorageEngine::wal_writer_thread_main()
{
    while (wal_running_)
    {
        std::unique_lock lock(wal_mutex_);

        // Wake thread if queue has no entries or when WAL is shutting down
        wal_cv_.wait(lock, [this] { return !wal_queue_.empty() || !wal_running_; });

        if (!wal_running_ && wal_queue_.empty()) break;

        // Batch-pop all entries to local vector to minimize lock hold
        std::vector<WalEntry> batch;
        batch.reserve(wal_queue_.size());
        while (!wal_queue_.empty())
        {
            batch.push_back(std::move(wal_queue_.front()));
            wal_queue_.pop_front();
        }

        // Notify flush_wal() if it is waiting for shutdown
        wal_cv_.notify_all();
        lock.unlock();

        // Write batch
        for (auto &entry : batch) {
            wal_manager_.append(entry.seq, entry.op, entry.key, entry.value);
        }
    }

    // Flush any remaining items before exit
    std::vector<WalEntry> remaining;
    {
        std::lock_guard lock(wal_mutex_);
        while (!wal_queue_.empty())
        {
            remaining.push_back(std::move(wal_queue_.front()));
            wal_queue_.pop_front();
        }
    }
    for (auto &entry : remaining)
        wal_manager_.append(entry.seq, entry.op, entry.key, entry.value);
}

void StorageEngine::snapshot_thread_main()
{
    while (snapshot_running_)
    {
        std::unique_lock lock(snapshot_mutex_);
        snapshot_cv_.wait(lock, [this] { return snapshot_requested_.load() || !snapshot_running_; });
        if (!snapshot_running_)
            break;
        snapshot_requested_ = false;
        lock.unlock();

        auto copy = kv_store_.data();
        // Determine last sequence to include in snapshot
        uint64_t last_seq = wal_manager_.next_sequence() - 1;

        bool saved = snapshot_manager_.save(copy, last_seq);
        if (!saved)
        {
            std::cerr << "Snapshot failed.\n";
        }
        else
        {
            // TODO : Implement WAL rotation
            std::cout << "Snapshot saved (seq=" << last_seq << ")\n";
        }
    }
}

void StorageEngine::put(const std::string& key, const std::string& value)
{
    kv_store_.put(key, value);

    uint64_t seq = wal_manager_.next_sequence();
    WalEntry entry{seq, "PUT", key, value};
    enqueue_wal(entry);
}

std::optional<std::string> StorageEngine::get(const std::string& key) const
{
    return kv_store_.get(key);
}

bool StorageEngine::remove(const std::string& key)
{
    bool existed = kv_store_.remove(key);
    if (existed) {
        uint64_t seq = wal_manager_.next_sequence();
        WalEntry entry{seq, "REMOVE", key, ""};
        enqueue_wal(entry);
    }
    return existed;
}

void StorageEngine::snapshot_async()
{
    // Signal snapshot thread
    snapshot_requested_ = true;
    snapshot_cv_.notify_one();
}

bool StorageEngine::snapshot_sync()
{
    // Create copy and save synchronously (useful for shutdown)
    auto copy = kv_store_.data();
    uint64_t last_seq = wal_manager_.next_sequence() - 1;
    return snapshot_manager_.save(copy, last_seq);
}

void StorageEngine::flush_wal()
{
    // Wait until queue empty
    std::unique_lock lock(wal_mutex_);
    wal_cv_.wait(lock, [this] { return wal_queue_.empty(); });
    lock.unlock();
    // TODO : Implement a barrier mechanism instead of sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void StorageEngine::shutdown(bool take_final_snapshot)
{
    // flush WAL queue
    flush_wal();

    // Take final snapshot if requested (synchronous to ensure persisted state)
    if (take_final_snapshot) {
        bool ok = snapshot_sync();
        if (!ok) std::cerr << "Warning: final snapshot failed\n";
    }

    // Stop background threads
    wal_running_ = false;
    wal_cv_.notify_one();

    snapshot_running_ = false;
    snapshot_cv_.notify_one();
}