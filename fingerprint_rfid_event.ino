#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 mfrc522(10, 9);  // SS_PIN, RST_PIN 설정

int Relay = 7;  // 릴레이 7번 핀 연결
int fingerFailCounter = 0;  // 지문 인식 실패 횟수 카운터
int RFIDFailCounter = 0;    // RFID 탐색 실패 횟수 카운터

// 등록된 카드 UID 리스트
String registered_cards[] = {
  "09 7F 71 8B",
  "A1 B2 C3 D4",  
  "12 34 56 78"  
};

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  delay(100);
  
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("지문 센서 연결 성공");
  } else {
    Serial.println("지문 센서 연결 실패");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  // 등록된 지문이 없는 경우
  if (finger.templateCount == 0) {
    Serial.print("등록된 지문이 없습니다. 지문 등록을 먼저 진행해주세요!");
  }
  // 등록된 지문이 1개 이상 있는 경우
  else {
    Serial.println("지문 인식 대기 중");
      Serial.print("등록된 지문 정보 "); Serial.print(finger.templateCount); Serial.println("개");
  }

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println("RFID 인식을 시작합니다");

  pinMode(Relay, OUTPUT); // 릴레이 핀 출력으로 설정
  digitalWrite(Relay, LOW); // 초기 릴레이 OFF
}

void loop() {
  // 지문 인식 처리
  if (getFingerprintID() == FINGERPRINT_NOFINGER) {
    // RFID 인식 처리
    checkRFID();
  }
  delay(50);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) {
    return p;
  }
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("지문 인식 성공");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("통신 오류 발생");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("지문 인식 오류 발생");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("지문 데이터 변환 성공");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("지문 이미지가 너무 지저분함");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("통신 오류 발생");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("지문 특징 확인 실패");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("지문 특징 확인 실패");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("일치하는 지문 탐색 완료!");
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    
    // 지문 일치 시 동작 코드 추가
    digitalWrite(Relay,HIGH);     // 릴레이 ON
    delay(1000);
    digitalWrite(Relay,LOW);      // 릴레이 OFF

    fingerFailCounter = 0;  // 지문 일치 시 실패 카운터 초기화
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("통신 오류 발생");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND || p != FINGERPRINT_OK) {
    // 지문을 인식했으나 일치하는 지문을 탐색 실패했을 때
    Serial.println("일치하는 지문 탐색 실패");
    fingerFailCounter++;  // 실패 시 카운터 증가
    if (fingerFailCounter >= 3) {  // 실패가 3회 이상이면 이벤트 발생
      Serial.println("지문 탐색 실패 3회 발생");
      fingerFailCounter = 0;  // 카운터 초기화
      delay(3000);
      Serial.println("지문 인식 대기 중");
    }
    return p;
  }
  delay(3000); 
  Serial.println("지문 인식 대기 중");

  return p;
}

void checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // 인식한 카드의 UID를 문자열로 저장
	Serial.print("UID tag: ");
  String tag_uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    tag_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  tag_uid.toUpperCase();
  Serial.println(tag_uid);

   // 등록된 카드인지 확인
  bool isRegistered = false;
  for (String registered_uid : registered_cards) {
    if (tag_uid.substring(1) == registered_uid) {
      isRegistered = true;
      break;
    }
  }

  if (isRegistered) {
    Serial.println("등록된 카드입니다.");

    // 등록된 카드 인식 시 이벤트 추가
    digitalWrite(Relay,HIGH);     // 릴레이 ON
    delay(1000);
    digitalWrite(Relay,LOW);      // 릴레이 OFF

    RFIDFailCounter = 0;  // UID 일치 시 실패 카운터 초기화
  } else {
    Serial.println("등록되지 않은 카드입니다.");
    RFIDFailCounter++;  // 실패 시 카운터 증가
    if (RFIDFailCounter >= 3) {  // 실패가 3회 이상이면 이벤트 발생
      Serial.println("RFID 탐색 실패 3회 발생 - 추가 이벤트 실행!");
      // 추가적인 이벤트 코드를 여기에 작성
      RFIDFailCounter = 0;  // 카운터 초기화
    }
  }

  delay(3000);
  Serial.println("RFID 인식 대기 중");
}
