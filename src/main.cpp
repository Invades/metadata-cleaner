#include "config.hpp"
#include "cleaner.hpp"
#include "file_watcher.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <csignal>

namespace {
    std::atomic<bool> running{true};
}

void signal_handler(int) {
    running = false;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "\nOptions:\n"
              << "  -c, --config <path>  Path to config file (default: config.json)\n"
              << "  -d, --directory <path>  Process directory once and exit\n"
              << "  -h, --help           Show this help message\n"
              << "  -v, --version        Show version\n";
}

int main(int argc, char* argv[]) {
    std::filesystem::path config_path = "config.json";
    std::optional<std::filesystem::path> one_time_dir;

    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        std::string arg = argv[arg_index];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "metadata-cleaner v" << PROJECT_VERSION << "\n";
            return 0;
        } else if ((arg == "-c" || arg == "--config") && arg_index + 1 < argc) {
            config_path = argv[++arg_index];
        } else if ((arg == "-d" || arg == "--directory") && arg_index + 1 < argc) {
            one_time_dir = argv[++arg_index];
        }
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    metacleaner::ConfigManager config_manager(config_path);
    if (!config_manager.load()) {
        std::cerr << "Failed to load config. Creating default config.\n";
        
        metacleaner::Config default_config;
        default_config.watch_directories = {"./music"};
        
        nlohmann::json j = default_config;
        std::ofstream out(config_path);
        out << j.dump(4);
        out.close();
        
        if (!config_manager.load()) {
            return 1;
        }
    }

    auto config = config_manager.get();
    metacleaner::Cleaner cleaner(config.dry_run, config.comment_pattern);

    if (one_time_dir.has_value()) {
        if (!std::filesystem::exists(one_time_dir.value())) {
            std::cerr << "Error: Directory does not exist: " << one_time_dir.value() << "\n";
            return 1;
        }

        std::cout << "Processing directory: " << one_time_dir.value() << "\n";
        auto results = cleaner.process_directory(one_time_dir.value(), config.recursive);
        
        int processed_count = 0;
        int error_count = 0;
        
        for (const auto& result : results) {
            if (result.success && result.modified) {
                processed_count++;
                std::cout << (config.dry_run ? "[DRY-RUN] Would remove comment: " : "Removed comment: ")
                          << result.file_path << "\n"
                          << "  Comment was: " << result.original_comment << "\n";
            } else if (!result.success && !result.error_message.empty() && result.error_message != "Unsupported format") {
                error_count++;
                std::cerr << "Error processing " << result.file_path << ": " << result.error_message << "\n";
            }
        }
        
        std::cout << "\nSummary: " << processed_count << " file(s) cleaned";
        if (error_count > 0) {
            std::cout << ", " << error_count << " error(s)";
        }
        std::cout << "\n";
        return 0;
    }

    // Watch mode
    metacleaner::FileWatcher watcher;

    auto update_watcher = [&](const metacleaner::Config& cfg) {
        watcher.clear_directories();
        for (const auto& dir : cfg.watch_directories) {
            watcher.add_directory(dir, cfg.recursive);
        }
        cleaner.set_dry_run(cfg.dry_run);
        cleaner.set_pattern(cfg.comment_pattern);
        std::cout << "Config reloaded.\n";
    };

    config_manager.set_change_callback(update_watcher);

    for (const auto& dir : config.watch_directories) {
        watcher.add_directory(dir, config.recursive);
    }

    watcher.set_callback([&cleaner, &config](const std::filesystem::path& path) {
        auto result = cleaner.process_file(path);
        if (result.success && result.modified) {
            std::cout << (config.dry_run ? "[DRY-RUN] Would remove comment: " : "Removed comment: ")
                      << path << "\n"
                      << "  Comment was: " << result.original_comment << "\n";
        } else if (!result.success && !result.error_message.empty() && result.error_message != "Unsupported format") {
            std::cerr << "Error processing " << path << ": " << result.error_message << "\n";
        }
    });

    std::cout << "Watching for changes.\n"
              << "Press Ctrl+C to stop.\n";

    while (running) {
        config_manager.check_for_changes();
        watcher.scan(running);

        const int scan_interval_ms = config.scan_interval_ms;
        const int sleep_chunk_ms = 100;
        const int iterations = scan_interval_ms / sleep_chunk_ms;

        for (int sleep_iteration = 0; sleep_iteration < iterations && running; ++sleep_iteration) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_chunk_ms));
        }
    }

    std::cout << "\nShutting down.\n";
    return 0;
}
