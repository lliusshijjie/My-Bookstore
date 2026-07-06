#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string read_file(const std::vector<std::string>& candidates)
{
    for (const auto& path : candidates) {
        std::ifstream file(path);
        if (!file.is_open()) continue;

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    return {};
}

void assert_contains(const std::string& text, const std::string& expected)
{
    assert(text.find(expected) != std::string::npos);
}

}  // namespace

int main()
{
    const auto compose = read_file({
        "../deploy/docker/docker-compose.yml",
        "deploy/docker/docker-compose.yml",
        "../../deploy/docker/docker-compose.yml",
    });
    assert(!compose.empty());
    assert_contains(compose, "healthcheck:");
    assert_contains(compose, "http://127.0.0.1:9006/api/ready");
    assert_contains(compose, "condition: service_healthy");

    const auto start_sh = read_file({"../start.sh", "start.sh", "../../start.sh"});
    assert(!start_sh.empty());
    assert_contains(start_sh, "/api/live");
    assert_contains(start_sh, "/api/ready");
    assert_contains(start_sh, "TINYWEBSERVER_ENABLE_REDIS=ON");
    assert_contains(start_sh, "ENABLE_HNSW=1");
    assert_contains(start_sh, "TINYWEBSERVER_ENABLE_HNSW=ON");

    const auto quick_start = read_file({
        "../scripts/start_server.sh",
        "scripts/start_server.sh",
        "../../scripts/start_server.sh",
    });
    assert(!quick_start.empty());
    assert_contains(quick_start, "/api/live");
    assert_contains(quick_start, "/api/ready");
    assert_contains(quick_start, "REDIS_URL=tcp://127.0.0.1:6379");

    const auto build_sh = read_file({
        "../scripts/build.sh",
        "scripts/build.sh",
        "../../scripts/build.sh",
    });
    assert(!build_sh.empty());
    assert_contains(build_sh, "ENABLE_REDIS=1");
    assert_contains(build_sh, "TINYWEBSERVER_ENABLE_REDIS");
    assert_contains(build_sh, "ENABLE_HNSW=1");
    assert_contains(build_sh, "TINYWEBSERVER_ENABLE_HNSW");

    const auto dockerfile = read_file({
        "../deploy/docker/Dockerfile",
        "deploy/docker/Dockerfile",
        "../../deploy/docker/Dockerfile",
    });
    assert(!dockerfile.empty());
    assert_contains(dockerfile, "ARG ENABLE_HNSW=0");
    assert_contains(dockerfile, "TINYWEBSERVER_ENABLE_HNSW=ON");

    const auto readme = read_file({"../README.md", "README.md", "../../README.md"});
    assert(!readme.empty());
    assert_contains(readme, "GET  /api/live");
    assert_contains(readme, "GET  /api/ready");
    assert_contains(readme, "TINYWEBSERVER_ENABLE_REDIS=ON");

    const auto api_doc = read_file({
        "../docs/ecommerce_api.md",
        "docs/ecommerce_api.md",
        "../../docs/ecommerce_api.md",
    });
    assert(!api_doc.empty());
    assert_contains(api_doc, "### GET /api/live");
    assert_contains(api_doc, "### GET /api/ready");
    assert_contains(api_doc, "503 Service Unavailable");

    std::cout << "test_deploy_runtime_contracts: all passed\n";
    return 0;
}
