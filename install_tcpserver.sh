#!/bin/bash

SERVICE_NAME=tcp_server
SERVICE_PATH=/etc/systemd/system/$SERVICE_NAME.service

# 현재 스크립트의 절대 경로를 기반으로 프로젝트 루트 계산
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# 실행파일 및 작업 디렉토리 경로
SERVER_EXEC_PATH="$PROJECT_ROOT/build/server"
WORKING_DIR="$PROJECT_ROOT/build"
LOG_FILE="$PROJECT_ROOT/build/${SERVICE_NAME}.log"

# 라이브러리 경로 설정
ENV_LD_LIBRARY_PATH="$PROJECT_ROOT/build"

# 1. 서버 실행파일 존재 확인
if [ ! -f "$SERVER_EXEC_PATH" ]; then
    echo "🚫 서버 실행 파일이 없습니다: $SERVER_EXEC_PATH"
    exit 1
fi

# 2. 로그 파일 생성 및 권한 설정
echo "🗂️ 로그 파일 생성 및 권한 설정: $LOG_FILE"
touch "$LOG_FILE"
chmod 644 "$LOG_FILE"

# 3. systemd 서비스 파일 생성
echo "📝 서비스 파일 생성 중: $SERVICE_PATH"
sudo tee $SERVICE_PATH > /dev/null <<EOF
[Unit]
Description=TCP Control Server Daemon
After=network.target

[Service]
ExecStart=$SERVER_EXEC_PATH
WorkingDirectory=$WORKING_DIR
Restart=always
User=root
Environment=LD_LIBRARY_PATH=$ENV_LD_LIBRARY_PATH
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# 3. 실행 권한 부여
chmod +x "$SERVER_EXEC_PATH"

# 4. systemd 리로드 및 서비스 등록
echo "🔄 systemd 리로드 및 서비스 등록"
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable $SERVICE_NAME
sudo systemctl restart $SERVICE_NAME

# 5. 상태 확인
echo "✅ 서비스 상태:"
sudo systemctl status $SERVICE_NAME --no-pager