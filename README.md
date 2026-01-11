# metadata-cleaner

A portable CLI application for automatically removing comment metadata from audio files (MP3, WAV, FLAC, ALAC/M4A).

Removes comments added by DRM-free music providers that don't contribute to actual artist information.

## Features

- Watches directories for new/modified audio files
- Removes all comment metadata unconditionally (or with optional regex pattern)
- Hot-reload configuration (no restart needed)
- Dry-run mode for testing
- Cross-platform (Linux, Windows, macOS)
- Docker support

## Supported Formats

- MP3
- FLAC
- WAV
- M4A (ALAC)

## Building

### Requirements

- CMake 3.20+
- Clang/Clang-cl
- Ninja

### Linux

```bash
cmake -B build -G "Ninja Multi-Config" \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang
cmake --build build --config Release
```

### macOS

```bash
# Install dependencies
brew install ninja

cmake -B build -G "Ninja Multi-Config" \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang
cmake --build build --config Release
```

### Windows

```bash
cmake -B build -G "Ninja Multi-Config" \
  -DCMAKE_CXX_COMPILER=clang-cl \
  -DCMAKE_C_COMPILER=clang-cl
cmake --build build --config Release
```

## Usage

### Watch Mode (default)

Continuously monitors directories for changes:

```bash
metadata-cleaner [options]

Options:
  -c, --config <path>      Path to config file (default: config.json)
  -d, --directory <path>   Process directory once and exit
  -h, --help               Show help message
  -v, --version            Show version
```

### One-time Run Mode

Process a directory once and exit:

```bash
# Process a directory once and exit
./metadata-cleaner -d /path/to/music

# With config file
./metadata-cleaner -c config.json -d /path/to/music
```

## Configuration

Configuration is stored in `config.json`:

**Remove all comments unconditionally:**

```json
{
    "watch_directories": ["/path/to/music"],
    "recursive": true,
    "dry_run": false,
    "scan_interval_ms": 5000
}
```

**Remove only comments matching a pattern:**

```json
{
    "watch_directories": ["/path/to/music"],
    "comment_pattern": ".*[Dd]ownloaded from.*",
    "recursive": true,
    "dry_run": false,
    "scan_interval_ms": 5000
}
```

| Option              | Description                                                                                                        |
| ------------------- | ------------------------------------------------------------------------------------------------------------------ |
| `watch_directories` | List of directories to monitor                                                                                     |
| `comment_pattern`   | Optional regex pattern - only matching comments are removed. If omitted, all comments are removed unconditionally. |
| `recursive`         | Scan subdirectories                                                                                                |
| `dry_run`           | Preview changes without modifying files                                                                            |
| `scan_interval_ms`  | How often to scan for changes (milliseconds)                                                                       |

Configuration changes are detected automatically at runtime.

## Docker

An example [docker-compose.yaml](docker-compose.yaml) configuration is provided.

```bash
docker-compose up -d
```

Mount your music directory and config in `docker-compose.yaml`:

```yaml
volumes:
  - ./config:/data
  - /path/to/music:/data/music:rw
```

## Acknowledgments

- [TagLib](https://taglib.org/) - Audio metadata library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
- [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) - CMake dependency management
