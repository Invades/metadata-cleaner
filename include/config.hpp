#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <nlohmann/json.hpp>

namespace metacleaner {
    struct Config {
        std::vector<std::string> watch_directories;
        std::optional<std::string> comment_pattern;
        bool recursive = true;
        bool dry_run = false;
        int scan_interval_ms = 5000;
    };

    void to_json(nlohmann::json& j, const Config& c);
    void from_json(const nlohmann::json& j, Config& c);

    class ConfigManager {
    public:
        using ChangeCallback = std::function<void(const Config&)>;

        explicit ConfigManager(const std::filesystem::path& config_path);
        ~ConfigManager();

        bool load();
        const Config& get() const { return config_; }
        void set_change_callback(ChangeCallback callback);
        void check_for_changes();

    private:
        std::filesystem::path config_path_;
        Config config_;
        std::filesystem::file_time_type last_modified_;
        ChangeCallback on_change_;
    };
}
