# 📡 TCP 기반 원격 제어 시스템
### 📁 프로젝트 개요
이 프로젝트는 TCP 서버–클라이언트 통신을 기반으로 한 원격 장치 제어 시스템입니다.
서버는 라즈베리파이에서 데몬으로 동작하며, 클라이언트의 명령을 받아 다음과 같은 장치들을 제어합니다:

```
🖥️ 서버: 라즈베리파이 4B

💡 클라이언트: Ubuntu
```
- LED (밝기 조절 포함)

- Buzzer (PWM 제어)

- Light Sensor (조도 측정)

- 7-Segment Display

### 🛠️ 구성  
```
[project]
   ├── [build]      → 빌드 산출물(.o, .so, 실행파일, 로그)  
   ├── [include]    → 헤더 파일(device_control.h)  
   ├── [src]        → 소스 파일(.c, .o)  
   ├── main.c       → 장치 제어 테스트용 main문  
   ├── Makefile     → 빌드 스크립트  
   ├── install_tcpserver.sh → 서버 데몬 설치 스크립트  
   └── readme.md → 문서  
```
   
### ⚙️ 주요 기능  

클라이언트로부터 명령 수신 후 장치 제어

공유 라이브러리를 통한 모듈화된 장치 제어

뮤텍스를 활용한 스레드 안전성 확보

서버 종료 시 안전한 자원 해제 및 로그 기록

데몬 형태로 백그라운드 실행 (root 권한 필요)

### 🖥️ 빌드 및 실행 방법  
*[1] Server*  
1. 빌드 전 clean
```bash
make clean
```
2. make: 빌드 시작
```bash
make
```
3. 서버 데몬 설치 (자동 등록)
```bash
chmod +x install_tcpserver.sh
./install_tcpserver.sh
```  
4. server 로그 확인  
```bash
cat build/tcp_server.log 
```  

*[2] Client* 
1. 빌드 전 clean
```bash
make clean
```
2. make: 빌드 시작
```bash
make
``` 
3. 클라이언트 실행
```bash
./build/client 192.168.0.41
# ./client [서버 IP]   
 
```  

### 🔒 주의사항
서버는 root 권한으로 실행해야 하며, 실행 전 GPIO 핀 설정 확인 필요

공유 라이브러리는 build/ 폴더 내 .so 파일로 제공됨

LED, Light Sensor, 7-Segment는 특정 GPIO 핀에 연결되어야 정상 동작함

클라이언트에서 포트 번호 맞춰서 넣어주는 것 확인

**서버 kill하고 싶다면, sudo systemctl stop tcp_server.service**

### 📚 향후 개선사항
스레드풀 도입을 통한 자원 효율 개선

입력 값 유효성 검증 강화

자원별 뮤텍스 세분화 및 교착상태 방지 로직 강화

로그 파일 관리 및 시각화 기능 추가

