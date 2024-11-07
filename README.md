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

-지문 센서·RFID 센서·도어락
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

-DB
