#include "config.hpp"
#include <fstream>
#include <iostream>

namespace metacleaner {
    void to_json(nlohmann::json& j, const Config& c) {
        j = nlohmann::json{
            {"watch_directories", c.watch_directories},
            {"recursive", c.recursive},
            {"dry_run", c.dry_run},
            {"scan_interval_ms", c.scan_interval_ms}
        };
        if (c.comment_pattern.has_value()) {
            j["comment_pattern"] = c.comment_pattern.value();
        }
    }

    void from_json(const nlohmann::json& j, Config& c) {
        if (j.contains("watch_directories")) {
            j.at("watch_directories").get_to(c.watch_directories);
        }
        if (j.contains("comment_pattern")) {
            c.comment_pattern = j.at("comment_pattern").get<std::string>();
        }
        if (j.contains("recursive")) {
            j.at("recursive").get_to(c.recursive);
        }
        if (j.contains("dry_run")) {
            j.at("dry_run").get_to(c.dry_run);
        }
        if (j.contains("scan_interval_ms")) {
            j.at("scan_interval_ms").get_to(c.scan_interval_ms);
        }
    }

    ConfigManager::ConfigManager(const std::filesystem::path& config_path)
        : config_path_(config_path) {}

    ConfigManager::~ConfigManager() = default;

    bool ConfigManager::load() {
        if (!std::filesystem::exists(config_path_)) {
            std::cerr << "Config file not found: " << config_path_ << "\n";
            return false;
        }

        try {
            std::ifstream file(config_path_);
            if (!file.is_open()) {
                std::cerr << "Failed to open config file: " << config_path_ << "\n";
                return false;
            }

            nlohmann::json j = nlohmann::json::parse(file);
            config_ = j.get<Config>();
            last_modified_ = std::filesystem::last_write_time(config_path_);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse config: " << e.what() << "\n";
            return false;
        }
    }

    void ConfigManager::set_change_callback(ChangeCallback callback) {
        on_change_ = std::move(callback);
    }

    void ConfigManager::check_for_changes() {
        if (!std::filesystem::exists(config_path_)) {
            return;
        }

        auto current_modified = std::filesystem::last_write_time(config_path_);
        if (current_modified != last_modified_) {
            if (load() && on_change_) {
                on_change_(config_);
            }
        }
    }
}
