- DB
  - mySQL, 서버 열기·테이블 구성·데이터 관리
	- localhost로 DB를 만들었으나, DB가 저장된 노트북에 항상 연결해놓고 개발이 불가하기에 외부 ip로 DB 접속이 가능하게 옮겼다.

- DB 초기 구성 예정 리스트
	- MySQL
	- 이름·권한은 DB에서 직접 관리, 지문 데이터는 정수형으로 저장
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


- 테이블 구성 수정

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
  - 수정(11/19)
  	- userinfo의 username, userpw의 사용성 의문으로 칼럼 삭제; 사용자 로그인/인증 방식으로 사용하려 했으나 사용자 매치의 개념까지는 불필요하므로 칼럼 필요하지 않음.
   	- uid_int의 삭제; uid는 string 형식으로 저장되어야 함. 


	- create table fingerp/rfid_confirm_time, fingerp/rfid_event(
confirm/event timestamp default current_timestamp,
door_status boolean not null);
	*각각 지문/rfid, 문열림/이벤트 칼럼명 동일하게 지정해둠.
	*지문과 rfid 센서에서 이벤트 발생 시 현재 시각을 찍어줌: db상에선 door_status의 값이 들어가면 현재 시각이 저장되거나, 아니면 시각과 문 상태 모두 직접 입력도 가능. YYYY-MM-DD HH:MM:SS의 형식으로 저장.

	- userinfo table
   	![userinfo](https://github.com/user-attachments/assets/63f4f617-accd-46ad-a6f8-0563738f7d9e)
   	- 차례로 fingerp_confirm_time/rfid_confirm_time/fingerp_event/rfid_event table
![스크린샷 2024-11-19 020851](https://github.com/user-attachments/assets/757502c6-1a71-4a1c-bb9b-3eb9fa03879d)

	- userinfo table 데이터 삽입 모습
![userinfo data](https://github.com/user-attachments/assets/b9c68c85-3c4b-4b47-b20d-1ae16c219178)
	- fingerp_confirm_time table 데이터 삽입 모습
![fingerp_confirm_time data](https://github.com/user-attachments/assets/7fc2eddd-e6f7-4fdc-95f0-7e517f5b30ad)
	- table구성도
![tables](https://github.com/user-attachments/assets/9d9534a7-f668-461a-8eac-671f7dca7d43)

	- 그러나 1개의 id당 지문 등 정보를 한 개만 등록 가능한 구조로 수정 및 재구성이 필요->가능함.

-수정(11/23)
   - fingerp/rfid_confirm_time, fingerp/rfid_event table을 병합하고, door_status부분을 공동으로 둔다; pwa와 라즈베리파이에서 현재의 문 열림 상태를 받아오기 위해서임.
   - null값은 모두 허용하고, 지문/rfid의 인증 성공/실패의 이벤트 발생(현재) 시각이 저장되는 특성은 유지한다.
	- 수정된 테이블
 	- 테이블 구성, 특성
	- ![table](https://github.com/user-attachments/assets/9357e637-043e-4ce4-aa34-294138fda3da)
 	- 데이터 삽입 예시(현재 시각이 삽입됨)
 	- ![insert](https://github.com/user-attachments/assets/2cdbfe5e-5baf-44ce-94c6-de98c871cd37)


-----
- PWA
	- OS 문제로 npm start 명령어의 오류
		- cross-env 사용으로 해결
		- ![cross-env추가](https://github.com/user-attachments/assets/f0f92ef0-4784-4065-ad46-b46c87fbaab5)
 		- ![cross-env 실행](https://github.com/user-attachments/assets/36d57d74-9225-423e-a1a8-3705a7f27f89)

	- 핸드폰에서 DB가 불러와지지 않음
		- index.ts에서 cors 수정
 		- ![cors](https://github.com/user-attachments/assets/3c3b60fc-0b8e-49ad-a8db-c9cc4559353a)

	- ip 동일 환경 확인, 각 api를 컴퓨터의 local ip로 변경해주었으나 핸드폰에서 열리지 않아서 테스트는 하지 못하였음.
 	- 단, 옮겨진 환경이 아닌 기존(처음 개발된 환경)에서는 핸드폰으로 연결이 가능하였음.
		 - ![api 변경](https://github.com/user-attachments/assets/fb9cb0b0-4af1-4758-9b1a-150b6cb8251a)
	    	 - ![ip](https://github.com/user-attachments/assets/ac48760e-d94e-424c-9175-f5f325b9789c)
    		 - ![env](https://github.com/user-attachments/assets/3b686326-bcdf-474f-be34-4647078ff0bf)
	 	- ![api_doorlock_ts](https://github.com/user-attachments/assets/b3377649-fb35-43b3-8ef2-037774b7dc19)
		- ![api_index_ts](https://github.com/user-attachments/assets/91d3f35d-5ee0-4278-84c0-279271283599)
		- ![연결실패](https://github.com/user-attachments/assets/916b2e53-8829-41ee-8754-8704b8dcb800)

