# 스마트 도어락
3조 wireless network project 

필요 모듈
- 도어락

- 카메라

- 지문 센서(JM-101B)
http://www.11st.co.kr/products/6847069563/share

- RFID 모듈(RFID-RC522)
https://ggugemall.com/product/detail.html?product_no=2736&cate_no=105&display_group=1&cafe_mkt=naver_ks&mkt_in=Y&ghost_mall_id=naver&ref=naver_open&NaPm=ct%3Dm2u81l3s%7Cci%3D5900af3eb473724fb71017ace40a0bbf6ad991b8%7Ctr%3Dslct%7Csn%3D7375944%7Chk%3Dc09b7b65e295530bfefb1fa0e8d96a9c682554ac


---

10/29~

- 아두이노와 라즈베리파이 통신
	- 라즈베리파이; 카메라 기능 필요(실시간 녹화, 스크린샷/타임라인 저장, 안면인식 기능은 가능하면 추가), openCV, DB에 이벤트 발생 결과 업로드.
	- 아두이노; 사용자의 지문/nfc태그를 통한 보안 해제 시도 n회 이상 실패 시 이벤트 발생(스크린샷/타임라인 저장)을 라즈베리파이에 요청. 내장된 ESP8266을 통해 라즈베리파이와 wifi통신.
- pwa
	- 인터페이스 구현
	- 사용 이벤트 발생 시 사용자에게 알림
-DB
	- mySQL, 서버 열기·테이블 구성·데이터 관리

- 지문 센서·RFID 센서·도어락
	- 센서 도착 시 연결(다음 주~), 도어락 표본? 교수님이 제공해주실 예정. 참고해서 정보 수집 및 프로젝트에 추가

- 결정해야 할 것들
	- DB에 업로드할 것들?
	- 아두이노와 DB의 통신?
	- 이벤트 발생; 타임라인 저장(실시간 영상의 무조건적인 저장 필요) vs 이벤트 발생 시 스크린샷 촬영

---

11/5~

- 아두이노에서 wifi통신을 이용해 DB에 직접 접근 성공
	- 임시적으로 firebase-realtime database로 진행, 추후(센서 도착 및 데이터 값 받게 되면) mysql로 변경 및 코드 작성 예정
   
- 지문인식
	- 인증 성공 시 문 열림·닫힘 (이벤트)정보는 데이터로 가져올 수 있을 듯 
	- 인증 실패 시 데이터 가져오기 힘들 것; 타이밍을 지정하는 게 아니라 loop문으로 돌아가기 때문에 n회 인증 실패 시 이벤트 발생 처리는 어려울 듯 
	- 사용자 정보 DB에 저장
		- 이름·권한은 DB에서 직접 관리 필요
 		- 지문 데이터는 정수형으로 저장해야함 ; n번에 사용자의 검지 등록, 센서가 인식해 지문 확인 시 n번 데이터와 일치하면 이벤트 발생
  	- 지문 등록 시 이름·권한 입력 후 자동 등록되는 방식은 다른 부분 구현 후 시도 예정.

- DB
	- MySQL
  		- ver: 8.0.37, OS: Windows, x86,32-bit, MSI Installer
  		- pw: , server name: MySQL80
   
   	- 이름·권한은 DB에서 직접 관리, 지문 데이터는 정수형으로 저장
  	- table 구성(임시)
  	  - 누가, 언제, 어떻게 문을 사용했는가?
		- event_id (pk) -> 이벤트를 구분하는 고유번호
  		- timestamp: 발생 시각 -> 이벤트 발생 날짜, 시각
  		- event_type: 문 열림/닫힘, 인증성공/실패, 무단접근?
  		- auth_method: 지문/NFC/원격(지문, nfc, 원격은 pwa를통한 원격 열림 닫힘)
  		- user_id (누가 시도했는지 등록된 사용자라면 알 수 있음)
  		- image_path: 보안이벤트 발생시 캡처(보안 문제 발생시 찍힌 사진 저장)
  		- fail_count: 인증 실패 횟수

  	  - Users(사용자 관리) -> 인증된 사용자들
  		- user_id (PK) : 각 사용자의 고유번호
  		- name : 사용자의 이름/별칭 ...
  		- role: 관리자/일반 -> 권한 구분
  		- fingerprint_data : 등록된 지문 정보, 정수형이어야 함. (기존에 저장된 정보와 인식된 정보가 일치하는가로 판단하기 때문.)
  		- nfc_tag_id : 등록된 NFC 정보

  	  - userNotifications(알림설정) -> 어느 상황에, 누구에게 알림을 보낼까?
  		- user_id (PK) : 알림을 받을 사용자 구분
  		- door_alert : 문 열림/닫힘 알림 여부
  		- auth_alert : 인증 성공/실패 알림 여부
  		- security_alert : 보안 이벤트 알림 여부
---
11/12~

- 모스키토 통신 성공
- 아두이노와 라즈베리파이 wifi
- 프론트엔드 json으로 진행하다가 html로 재진행...
