#!/bin/bash
set -e

echo "🚗 1. osrm-extract 시작..."
sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend \
  osrm-extract -p /opt/car.lua /data/south-korea-latest.osm.pbf || { echo "❌ osrm-extract failed"; exit 1; }

echo "🧩 2. osrm-partition 시작..."
sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend \
  osrm-partition /data/south-korea-latest.osrm || { echo "❌ osrm-partition failed"; exit 1; }

echo "🎯 3. osrm-customize 시작..."
sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend \
  osrm-customize /data/south-korea-latest.osrm || { echo "❌ osrm-customize failed"; exit 1; }

echo "✅ 완료! osrm-routed 실행 준비됨"
