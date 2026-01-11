#include "file_watcher.hpp"

namespace metacleaner {
    void FileWatcher::add_directory(const std::filesystem::path& dir_path, bool recursive) {
        watch_entries_.push_back({dir_path, recursive});
    }

    void FileWatcher::clear_directories() {
        watch_entries_.clear();
        file_times_.clear();
    }

    void FileWatcher::set_callback(FileCallback callback) {
        on_file_changed_ = std::move(callback);
    }

    void FileWatcher::scan(const std::atomic<bool>& should_continue) {
        for (const auto& entry : watch_entries_) {
            if (!should_continue) {
                return;
            }

            if (!std::filesystem::exists(entry.path)) {
                continue;
            }

            auto process_file = [&](const std::filesystem::directory_entry& dir_entry) {
                if (!should_continue) {
                    return;
                }

                if (!dir_entry.is_regular_file()) {
                    return;
                }

                auto path_str = dir_entry.path().string();
                auto current_time = dir_entry.last_write_time();

                auto it = file_times_.find(path_str);
                if (it == file_times_.end()) {
                    file_times_[path_str] = current_time;
                    if (on_file_changed_) {
                        on_file_changed_(dir_entry.path());
                    }
                } else if (it->second != current_time) {
                    it->second = current_time;
                    if (on_file_changed_) {
                        on_file_changed_(dir_entry.path());
                    }
                }
            };

            if (entry.recursive) {
                for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(entry.path)) {
                    if (!should_continue) {
                        return;
                    }
                    process_file(dir_entry);
                }
            } else {
                for (const auto& dir_entry : std::filesystem::directory_iterator(entry.path)) {
                    if (!should_continue) {
                        return;
                    }
                    process_file(dir_entry);
                }
            }
        }
    }
}
