#include "cleaner.hpp"
#include <fileref.h>
#include <tag.h>
#include <algorithm>
#include <iostream>
#include <regex>

namespace metacleaner {
    Cleaner::Cleaner(bool dry_run, std::optional<std::string> pattern)
        : dry_run_(dry_run), pattern_(std::move(pattern)) {}

    void Cleaner::set_dry_run(bool dry_run) {
        dry_run_ = dry_run;
    }

    void Cleaner::set_pattern(std::optional<std::string> pattern) {
        pattern_ = std::move(pattern);
    }

    bool Cleaner::is_supported_format(const std::filesystem::path& file_path) const {
        static const std::vector<std::string> supported = {".mp3", ".wav", ".flac", ".m4a"};
        auto ext = file_path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return std::find(supported.begin(), supported.end(), ext) != supported.end();
    }

    bool Cleaner::should_remove_comment(const std::string& comment) const {
        if (comment.empty()) {
            return false;
        }

        if (!pattern_.has_value()) {
            return true;
        }

        try {
            std::regex re(pattern_.value(), std::regex::icase);
            return std::regex_search(comment, re);
        } catch (const std::regex_error&) {
            return comment.find(pattern_.value()) != std::string::npos;
        }
    }

    CleanResult Cleaner::process_file(const std::filesystem::path& file_path) {
        CleanResult result;
        result.file_path = file_path;
        result.success = false;
        result.modified = false;

        if (!is_supported_format(file_path)) {
            result.error_message = "Unsupported format";
            return result;
        }

        try {
            TagLib::FileRef file(file_path.c_str());
            if (file.isNull() || !file.tag()) {
                result.error_message = "Failed to open file or read tags";
                return result;
            }

            auto* tag = file.tag();
            std::string comment = tag->comment().to8Bit(true);
            result.original_comment = comment;

            if (should_remove_comment(comment)) {
                if (!dry_run_) {
                    tag->setComment(TagLib::String());
                    if (!file.save()) {
                        result.error_message = "Failed to save file";
                        return result;
                    }
                }
                result.modified = true;
            }

            result.success = true;
        } catch (const std::exception& e) {
            result.error_message = e.what();
        }

        return result;
    }

    std::vector<CleanResult> Cleaner::process_directory(const std::filesystem::path& dir_path, bool recursive) {
        std::vector<CleanResult> results;

        if (!std::filesystem::exists(dir_path)) {
            return results;
        }

        auto process_entry = [&](const std::filesystem::directory_entry& entry) {
            if (entry.is_regular_file() && is_supported_format(entry.path())) {
                results.push_back(process_file(entry.path()));
            }
        };

        if (recursive) {
            std::error_code ec;
            for (std::filesystem::recursive_directory_iterator it(dir_path, std::filesystem::directory_options::skip_permission_denied, ec);
                 it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
                if (ec) {
                    ec = std::error_code();
                    continue;
                }
                try {
                    process_entry(*it);
                } catch (const std::filesystem::filesystem_error&) {
                    // skip files that cause fs errors
                }
            }
        } else {
            std::error_code ec;
            for (std::filesystem::directory_iterator it(dir_path, ec);
                 it != std::filesystem::directory_iterator(); it.increment(ec)) {
                if (ec) {
                    ec = std::error_code();
                    continue;
                }
                try {
                    process_entry(*it);
                } catch (const std::filesystem::filesystem_error&) {

                }
            }
        }

        return results;
    }
}
