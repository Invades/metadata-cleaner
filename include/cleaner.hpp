#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace metacleaner {
    struct CleanResult {
        std::filesystem::path file_path;
        bool success;
        bool modified;
        std::string original_comment;
        std::string error_message;
    };

    class Cleaner {
    public:
        explicit Cleaner(bool dry_run = false, std::optional<std::string> pattern = std::nullopt);

        CleanResult process_file(const std::filesystem::path& file_path);
        std::vector<CleanResult> process_directory(const std::filesystem::path& dir_path, bool recursive);

        void set_dry_run(bool dry_run);
        void set_pattern(std::optional<std::string> pattern);

    private:
        bool should_remove_comment(const std::string& comment) const;
        bool is_supported_format(const std::filesystem::path& file_path) const;

        bool dry_run_;
        std::optional<std::string> pattern_;
    };
}
