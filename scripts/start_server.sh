#!/bin/bash
pkill -x server 2>/dev/null
sleep 1
cd /mnt/d/my_project/TinyWebServer || exit 1
REDIS_URL=tcp://127.0.0.1:6379 nohup ./build/server > /tmp/tws_server.log 2>&1 &
sleep 2
echo "PID=$(pgrep -x server)"
ss -ltn 2>/dev/null | grep ':9006' || echo "no-listener"
curl -s -o /dev/null -w "live=HTTP %{http_code}\n" http://127.0.0.1:9006/api/live
curl -s -o /dev/null -w "ready=HTTP %{http_code}\n" http://127.0.0.1:9006/api/ready
echo "--- log tail ---"
tail -n 8 /tmp/tws_server.log
