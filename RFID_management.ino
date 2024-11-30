#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(10, 9);  // SS_PIN, RST_PIN 설정

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  Serial.println("카드 관리 시스템 시작");
  Serial.println("\n1. 카드 등록, 2. 카드 삭제");
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  if (Serial.available()) {
    int command = readnumber(); // 입력값 읽기  
    // 명령어에 따라 동작 분기
    switch (command) {
      case 1: // 카드 등록
        Serial.println("카드 등록 프로세스 시작");
        Serial.println("등록하실 카드를 RFID 센서에 인식시켜주세요.");
        registerCard();
        break;

      case 2: // 카드 삭제
        Serial.println("카드 삭제 프로세스 시작");
        Serial.println("삭제하실 카드를 RFID 센서에 인식시켜주세요.");
        deleteCard();
        break;

      default: // 잘못된 입력 처리
        Serial.println("잘못된 명령입니다. 1~2 사이의 번호를 입력하세요.");
        break;
    }
    Serial.println("\n1. 카드 등록, 2. 카드 삭제");
  }
}

// 카드 등록 함수
void registerCard() {
  String tag_uid = "";
  // 카드가 인식될 때까지 checkRFID()를 반복 호출
  while (tag_uid == "") {
    tag_uid = checkRFID();  // tag_uid 값이 빈 문자열일 경우 계속 반복
    delay(100);
  }
  // 카드가 인식되면 tag_uid 값 출력
  Serial.print("등록할 카드 UID:");
  Serial.println(tag_uid);
  // RFID 데이터 삽입 SQL 코드 추가
}

// 카드 삭제 함수
void deleteCard() {
  String tag_uid = "";
  // 카드가 인식될 때까지 checkRFID()를 반복 호출
  while (tag_uid == "") {
    tag_uid = checkRFID();  // tag_uid 값이 빈 문자열일 경우 계속 반복
    delay(100);
  }
  // 카드가 인식되면 tag_uid 값 출력
  Serial.print("삭제할 카드 UID: ");
  Serial.println(tag_uid);
  // RFID 데이터 삭제 SQL 코드 추가
}

// 카드 인식 후 인식한 카드의 UID 반환
String checkRFID() {
  String tag_uid = "";
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return tag_uid;
  }
  // 인식한 카드의 UID를 문자열로 저장
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    tag_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tag_uid.toUpperCase();
  return tag_uid;
}