#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPOSE_FILE="${ROOT_DIR}/deploy/docker/docker-compose.yml"
SERVER_PORT="${SERVER_PORT:-9006}"

cleanup() {
    if [[ "${KEEP_STACK:-0}" != "1" ]]; then
        docker compose -f "${COMPOSE_FILE}" down
    fi
}
trap cleanup EXIT

docker compose -f "${COMPOSE_FILE}" up -d --build mysql inventory-service server

for _ in $(seq 1 40); do
    if curl -fsS "http://127.0.0.1:${SERVER_PORT}/api/health" >/dev/null; then
        break
    fi
    sleep 2
done

before_json="$(curl -fsS "http://127.0.0.1:${SERVER_PORT}/api/inventory/books/1")"
order_json="$(curl -fsS \
    -H "Content-Type: application/json" \
    -X POST \
    -d '{"user_id":2,"items":[{"book_id":1,"quantity":1}]}' \
    "http://127.0.0.1:${SERVER_PORT}/api/orders")"
after_json="$(curl -fsS "http://127.0.0.1:${SERVER_PORT}/api/inventory/books/1")"

python3 - "$before_json" "$order_json" "$after_json" <<'PY'
import json
import sys

before = json.loads(sys.argv[1])
order = json.loads(sys.argv[2])
after = json.loads(sys.argv[3])

before_stock = before["data"]["available"]
after_stock = after["data"]["available"]

assert order["code"] == 0, order
assert after_stock == before_stock - 1, (before_stock, after_stock)

print(f"inventory gRPC e2e passed: book 1 stock {before_stock} -> {after_stock}")
PY
