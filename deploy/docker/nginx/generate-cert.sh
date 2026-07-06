#!/bin/bash
# 生成本地自签 TLS 证书（仅用于开发/测试）
# 生产环境请替换为正式证书，例如 Let's Encrypt:
#   certbot certonly --webroot -w /var/www/certbot -d your.domain

set -e

CERT_DIR="$(cd "$(dirname "$0")" && pwd)/certs"
mkdir -p "$CERT_DIR"

if [ -f "$CERT_DIR/fullchain.pem" ] && [ -f "$CERT_DIR/privkey.pem" ]; then
    echo "certs already exist in $CERT_DIR, skip."
    exit 0
fi

echo "[cert] generating self-signed certificate into $CERT_DIR ..."

openssl req -x509 -nodes -newkey rsa:2048 -days 825 \
    -keyout "$CERT_DIR/privkey.pem" \
    -out    "$CERT_DIR/fullchain.pem" \
    -subj   "/C=CN/ST=Local/L=Local/O=TinyWebServer/OU=Dev/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"

chmod 600 "$CERT_DIR/privkey.pem"
echo "[cert] done. files:"
ls -l "$CERT_DIR"
