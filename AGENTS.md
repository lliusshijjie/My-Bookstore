# Repository Guidelines

## Project Structure & Module Organization

This is a Linux-oriented C++17 TinyWebServer implementation. Core startup and orchestration live in `main.cpp`, `config.*`, and `webserver.*`. HTTP parsing, routing, static file serving, and user-cache logic are in `http/`. The thread pool is header-only under `threadpool/`; timers are in `timer/`; logging is in `log/`; MySQL pooling is in `CGImysql/`; POSIX synchronization wrappers are in `lock/`. Static web assets and demo pages are served from `root/`. Pressure-test tooling is under `test_pressure/webbench-1.5/`.

## Build, Test, and Development Commands

- `sh ./build.sh`: build `server` with the Makefile in Debug mode.
- `DEBUG=0 sh ./build.sh`: build optimized Makefile output.
- `make server`: compile the server directly with `g++ -std=c++17`.
- `make clean`: remove the generated `server` binary.
- `sh ./build.sh cmake`: configure and build via CMake in `build/`.
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)`: explicit CMake release build.
- `./server -p 9006`: run locally; configure MySQL credentials in `main.cpp` first.

The build links `pthread` and `mysqlclient`; install MySQL development headers before building on a clean Linux host.

## Coding Style & Naming Conventions

Use C++17, four-space indentation, and existing brace style: function braces on the next line, class braces on the same line. Prefer RAII, `std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic`, `std::string_view`, and `enum class` where the current code already does. Classes use PascalCase (`HttpConn`, `WebServer`), constants use `k`-prefixed PascalCase (`kReadBufSize`), and private members commonly use a trailing underscore (`sockfd_`). Keep module-specific code inside its existing directory.

## Testing Guidelines

There is no dedicated unit-test framework. Validate changes by building, running `./server`, and exercising login, registration, and static file paths through a browser or `curl`. For concurrency or performance-sensitive changes, use `test_pressure/webbench-1.5/webbench`, for example `./webbench -c 1000 -t 5 http://127.0.0.1:9006/`.

## Commit & Pull Request Guidelines

Recent commits use short Conventional Commit-style subjects such as `fix: resolve critical bugs found in code review` and `refactor: modernize http_conn with C++17`. Follow `type: imperative summary` for new commits, with types like `fix`, `refactor`, `docs`, or `build`. Pull requests should describe the touched modules, include build/test evidence, note any MySQL or runtime configuration changes, and mention pressure-test results when behavior affects epoll, timers, logging, or thread-pool code.

## Security & Configuration Tips

Do not commit real database credentials. Keep generated logs and the `server` binary out of review unless the change specifically concerns those artifacts. When modifying request parsing or CGI login/register handling, preserve SQL escaping and bounds checks.
