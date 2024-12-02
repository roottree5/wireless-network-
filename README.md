# 스마트 도어락 시스템 프로젝트_신현우

## 역할 및 작업 내용
---
**MQTT통신 코드 및 설정**

**목표**

MQTT통신을 위한 broker의 설치와 설정
아두이노에서의 MQTT Subscribe 코드
라즈베리파이에서의 MQTT MQTT sub pub 코드
PWA파트에서의 원격제어를 위한 MQTT Publish 코드

## 진행 내용

라즈베리 파이를 MQTT broker로 세팅하고 아두이노, PWA의 웹/앱 서비스와 MQTT를 통한 통신을 하도록 코드를 작성하도록 하였습니다


## **아두이노의 MQTT broker 설치와 설정**

아두이노를 MQTT의 브로커로 설정하고 MQTT broker를 정하였습니다

[MQTT server support](https://github.com/mqtt/mqtt.org/wiki/server-support#capabilities) 에서 정리된 broker들을 확인 할수 있었습니다.

이중 오픈소스이며 설치, 사용이 간단하고 가볍고 OS이식성이 좋아 다양한 사용 예제들이 있는 Mosquitto로 정하였습니다.

모스키토의 프로그램 인증키를 다운로드하고
```
cd ~
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
```

모스키토의 저장소 패키지를 등록합니다
```
cd /etc/apt/source.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-stretch.list
```

이후 모스키토를 설치합니다
```
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
```

모스키토를 실행합니다
```
cd /etc/mosquitto
mosquitto -c mosquitto.conf -v -d
```

## **아두이노와 PWA에 broker에 연결하는 코드 작성**
- 


## ***설정 중 발생한 문제***
-  브로커 접속 문제
  - starting in local only mode와 함께 외부 접속이 되지않음
    - 해결 방안
    	- 설치직후 MQTT 실행전 conf를 설정합니다
	  ```
	  sudo vi /etc/mosquitto/mosquitto.conf
	  ```
	
	  내부에 mosquitto.conf 파일에 다음 내용을 추가합니다
	  ```
	  listener 1883
	  allow_anonymous ture
	  ```
  - mosquitto unable to write pid file 내용으로 mosquitto.conf 파일이 적용되지 않음
    - 해결 방안
      - 사용자계정을 root로 하여 실행
        ```
        sudo su - root
        mosquitto -c mosquitto.conf -v -d
        ```
  - broker에 접속이 되지 않는문제 
  - 라즈베리에서의 publish와 subscribe의 모든 기능은 작동하였으나 같은 네트워크의 아두이노,휴대폰,노트북에서의 접속이 불가함
    - 해결 방안
      - 홈 네트워크 설정을 했던 내용 중 통신정책설정과 포트포워딩 설정을 복구함
