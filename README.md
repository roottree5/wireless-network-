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
		-라즈베리파이; 카메라 기능 필요(실시간 녹화, 스크린샷/타임라인 저장, 안면인식 기능은 가능하면 추가), openCV, DB에 이벤트 발생 결과 업로드.
		-아두이노; 사용자의 지문/nfc태그를 통한 보안 해제 시도 n회 이상 실패 시 이벤트 발생(스크린샷/타임라인 저장)을 라즈베리파이에 요청. 내장된 ESP8266을 통해 라즈베리파이와 wifi통신.
	- pwd
		-인터페이스 구현
		-사용 이벤트 발생 시 사용자에게 알림
	-DB
		-mySQL, 서버 열기·테이블 구성·데이터 관리

	-지문 센서·RFID 센서·도어락
		-센서 도착 시 연결(다음 주~), 도어락 표본? 교수님이 제공해주실 예정. 참고해서 정보 수집 및 프로젝트에 추가