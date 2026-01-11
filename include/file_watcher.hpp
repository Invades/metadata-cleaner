#pragma once

#include <filesystem>
#include <functional>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>

namespace metacleaner {
    class FileWatcher {
    public:
        using FileCallback = std::function<void(const std::filesystem::path&)>;

        FileWatcher() = default;

        void add_directory(const std::filesystem::path& dir_path, bool recursive);
        void clear_directories();
        void set_callback(FileCallback callback);
        void scan(const std::atomic<bool>& should_continue);

    private:
        struct WatchEntry {
            std::filesystem::path path;
            bool recursive;
        };

        std::vector<WatchEntry> watch_entries_;
        std::unordered_map<std::string, std::filesystem::file_time_type> file_times_;
        FileCallback on_file_changed_;
    };
}
