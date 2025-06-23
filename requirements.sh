#!/bin/bash

set -e

echo "👉 Docker 설치를 시작합니다..."

# 1. 기존 docker 제거 (선택)
sudo apt-get remove -y docker docker-engine docker.io containerd runc || true

# 2. 필수 패키지 설치
sudo apt-get update
sudo apt-get install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

# 3. GPG 키 저장 디렉토리 생성
sudo install -m 0755 -d /etc/apt/keyrings

# 4. Docker 공식 GPG 키 다운로드
curl -fsSL https://download.docker.com/linux/debian/gpg | \
    sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# 5. 저장소 추가
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
  https://download.docker.com/linux/debian \
  $(lsb_release -cs) stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 6. 패키지 인덱스 업데이트
sudo apt-get update

# 7. Docker 설치
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# 8. Docker 서비스 상태 확인
echo "✅ Docker 설치 완료"
sudo docker --version

sudo apt-get install libcurl4-openssl-dev # curl

sudo apt-get install git g++ cmake libboost-dev libboost-filesystem-dev libboost-thread-dev \
libboost-system-dev libboost-regex-dev libxml2-dev libsparsehash-dev libbz2-dev \
zlib1g-dev libzip-dev libgomp1 liblua5.3-dev \
pkg-config libgdal-dev libboost-program-options-dev libboost-iostreams-dev \
libboost-test-dev libtbb-dev libexpat1-dev