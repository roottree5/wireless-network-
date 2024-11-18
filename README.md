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
- DB
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

- 오류사항들도 기록해놓을 것
- 모스키토 통신 성공
- 아두이노
	- 로컬 네트워크(아두이노와 mysql서버가 같은 공유기 사용 시)에는 연결 ○, 다른 공유기 사용하는 외부 네트워크에서는 구현 안 됨
 	- 태스트 진행은 임의로 user, database만들어 진행
  	- 아두이노에서 mysql에 쿼리문으로 컨트롤까지 구현 완료
  	  
- mqtt통신
	- 로컬 네티워크에서 mqtt를 통한 라즈베리파이와 pc통신 구현 완료
  	- 라즈베리파이에서 파이썬 코드를 사용한 데이터 확인 및 가공 가능
  	- 아두이노에서의 mqtt연결 코드; 데이터 송·수신 코드 작성 진행 중

- 지문인식/RFID
	- 통합 모듈 작동 부분 구현 완료
 		- 지문인식, RFID모듈 동시 작동과 등록된 지문 OR 카드 인식 시 릴레이 모듈에 신호 발생시켜 작동하는 것까지 구현 완료
   	- 지문 등록과 삭제, 카드 등록과 삭제 한 코드로 실행 가능하도록 구현 중
   	- wifi 아두이노에 코드, 센서 이식 및 DB에 데이터 전송하는 코드 추가 예정, 어느 정도 마무리 될 듯.
   	  
   	- ![통합모듈작동구현완료](https://github.com/user-attachments/assets/ce269f5f-44e4-44f3-aa08-23b20e1d96e3)
   	- 통합 모듈 부분 구현 완료


- 프론트엔드
	- react로 초안 진행, html로 재구성 중
 	- API전달(기능), 디자인 등 세분화해서 진행 예정
  	- PWA-DB통신 구현은 합동 진행
  	- 각 화면별로 더미 데이터 넣어서 프로토타입 구현 완료(기기와 DB의 연동이 완료되면 본격적인 데이터 입력해 UI수정 예정)
	- <img width="1511" alt="KakaoTalk_20241114_220042425_04" src="https://github.com/user-attachments/assets/96cef1c0-4862-46e6-a4f5-863e53a0c0e4">
	 <img width="281" alt="KakaoTalk_20241114_220042425_03" src="https://github.com/user-attachments/assets/0e8ccddb-9372-49c8-ab24-4d573749122f">
	<img width="268" alt="KakaoTalk_20241114_220042425_02" src="https://github.com/user-attachments/assets/25ee612e-06b0-4755-9ba6-ae4a7e66d412">
	 <img width="359" alt="KakaoTalk_20241114_220042425_01" src="https://github.com/user-attachments/assets/99e9319d-01c7-47c4-90b1-d6b08c8aa62b">
	<img width="1507" alt="KakaoTalk_20241114_220042425" src="https://github.com/user-attachments/assets/d9eb7bda-a564-479f-a2c7-d65f7f83f877">
	- 모바일 화면에서도 로컬로 접속해 UI 테스트까지 완료
![KakaoTalk_20241114_220049780_01](https://github.com/user-attachments/assets/f2768fdb-2df7-46df-8b8b-ff950a97ba2a)
![KakaoTalk_20241114_220049780](https://github.com/user-attachments/assets/58b248cf-4937-401b-b638-170b20899713)


- DB
	- 테이블 구성
 	- DB: sensor_data, tables: userinfo, fingerp_confirm_time, rfid_confirm_time, fingerp_event, rfid_event. 총 5개의 table.
	- use sensor_data;
	- create table userinfo(
username varchar(20) not null,
userpw varchar(30) not null,
fid int not null,
uid varchar(20) not null,
uid_int int not null auto_increment primary key,
get_alert boolean not null);
	*uid(rfid정보 저장-사용자 아이디가 될 것) 일단 두 개의 칼럼을 만들어둠. 정수형/자동 증가, 혹은 문자형으로 일반 아이디처럼 사용하도록.. 아직 확정되지 않음.
	- primary key 삭제: ALTER TABLE table name(userinfo) DROP PRIMARY KEY;
 	- 기존 칼럼에 primary key 특성 추가: ALTER TABLE table_name ADD PRIMARY KEY (column_name);


	- create table fingerp/rfid_confirm_time, fingerp/rfid_event(
confirm/event timestamp default current_timestamp,
door_status boolean not null);
	*각각 지문/rfid, 문열림/이벤트 칼럼명 동일하게 지정해둠.
	*지문과 rfid 센서에서 이벤트 발생 시 현재 시각을 찍어줌: db상에선 door_status의 값이 들어가면 현재 시각이 저장되거나, 아니면 시각과 문 상태 모두 직접 입력도 가능. YYYY-MM-DD HH:MM:SS의 형식으로 저장.

	- userinfo table
   	![userinfo](https://github.com/user-attachments/assets/63f4f617-accd-46ad-a6f8-0563738f7d9e)
   	- 차례로 fingerp_confirm_time/rfid_confirm_time/fingerp_event/rfid_event tale
![스크린샷 2024-11-19 020851](https://github.com/user-attachments/assets/757502c6-1a71-4a1c-bb9b-3eb9fa03879d)

	- userinfo table 데이터 삽입 모습
![userinfo data](https://github.com/user-attachments/assets/b9c68c85-3c4b-4b47-b20d-1ae16c219178)
	- fingerp_confirm_time table 데이터 삽입 모습
![fingerp_confirm_time data](https://github.com/user-attachments/assets/7fc2eddd-e6f7-4fdc-95f0-7e517f5b30ad)


	- 공동 칼럼을 door_status라 할 때, 쿼리문 참고(join)
SELECT fingerp_confirm_time.confirm
FROM userinfo
JOIN fingerp_confirm_time ON userinfo.door_status = fing_confirm_time.event
WHERE userinfo.uid = 5;



~ 11/18 ( pwa 통신 구현 계획(임시), 추가할 내용이 있는데 이것은 회의를 통해 기재할 예정.)
백엔드(node.js , express(프레임워크)에서 사용할 axios라는 http 클라이언트 라이브러리를 설치하여
mysql에서 데이터베이스를 가져올 때 json 형식으로 자동변환해서 그 데이터를 사용하도록 할거고, 
socket.io는 websocket(실시간 양방향 통신)을 구현할 때 쉽게 구현하게 해주는 라이브러리 입니다.

일반적인 데이터 조회 ( rest api , axios ) , 실시간 데이터 업데이트 ( socket.io )

서버 접속해서 mysql에서 데이터베이스를 가져올 때 rest api 형식을 사용할 것임. 이건 일반적인 데이터 조회/저장 할때 사용하는 것, 실시간 업데이트가 필요할 때는 Websocket 통신으로 실시간 양방향 통신을 구현할 것임..
rest api는 예를 들어 기존 출입 기록 같은걸 가져올 떄 사용할 것 같음. ( 데이터를 주고 받을 때 사용 )
Websocket은 예를 들어 새로운 지문정보가 등록 됐을 때 바로 화면에 표시하는 방식의 중개자 역할을 함.

정리: 
MySQL -> REST API(서버) -> axios(PWA) -> 화면 표시 ( 데이터 조회할 떄 )
이벤트 발생 -> WebSocket(서버) -> socket.io(PWA) -> 실시간 화면 업데이트 ( 새로운 이벤트가 발생했을 때 )

데이터베이스 작업이 마무리 되면 접속 정보(서버 ip 같은거)를 전달 받고 mysql 연결 설정을 하고(데이터베이스 접속 정보 서버 ip 같은거) rest api (데이터 주고받기 작업)을 하고
websocket(실시간 양방향 통신)을 하면서
pwa에서 데이터 표시를 할건데,
axios로 서버에 데이터를 요청해서 json 형식 자동변환으로 받아와서 받은 데이터를 화면에 표시하고 websocket으로 실시간 업데이트를 함.





 
