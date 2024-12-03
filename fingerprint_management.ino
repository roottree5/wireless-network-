//지문 등록, 삭제관리 

#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>





#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(D4, D1);
#else
#define mySerial Serial1
#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int id;

// Wi-Fi 정보
const char* ssid = "TP-Link 지율";
const char* password = "sks113301";

// MySQL 서버 정보
IPAddress server_addr(183, 97, 149, 51);  // MySQL 서버 IP 주소
char user[] = "test";                    // MySQL 사용자명
char pass[] = "89352183";                // MySQL 비밀번호

// MySQL 연결 객체 생성
WiFiClient client;
MySQL_Connection conn(&client);

bool wifiConnected = false;
bool mysqlConnected = false;




void setup() {
  Serial.begin(9600);
  delay(5000);
  while (!Serial);  
  Serial.println("시리얼모니터연결 성공");

  delay(100);
  finger.begin(57600);
  delay(100);
  if (finger.verifyPassword()) {
    Serial.println("지문 센서 연결 성공");
  } else {
    Serial.println("지문 센서 연결 실패"); 
    while (1) { delay(1); } //연결 실패시 무한루프 돌면서 연결대기
  }

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  if (!wifiConnected) {
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
      delay(500);
      Serial.print(".");
      retryCount++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected successfully!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      wifiConnected = true;
    } else {
      Serial.println("WiFi connection failed!");
      return;
    }
  }

  // MySQL 연결 확인
  if (!mysqlConnected) {
    Serial.println("Attempting to connect to MySQL server...");
    if (conn.connect(server_addr, 3306, user, pass)) {
      Serial.println("Connected to MySQL server.");
      mysqlConnected = true;
    } else {
      Serial.println("MySQL connection failed.");
      return;
    }
  }

  Serial.println("지문 관리 시스템 시작");
  Serial.println("\n1. 지문 등록, 2. 지문 삭제");
}

//serial로 입력된 값 반환
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
      case 1: // 지문 등록
        Serial.println("지문 등록 프로세스 시작");
        Serial.println("1~127 중 지문 정보를 저장하고 싶은 ID를 입력해주세요.");
        id = readnumber();

        if (mysqlConnected) {
            MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
            cursor->execute("USE sensor_data");

            // ID 존재 확인 쿼리
            String checkQuery = "SELECT COUNT(*) AS count FROM userinfo WHERE fid = " + String(id);
            cursor->execute(checkQuery.c_str());
            
            // 결과 확인
            column_names *columns = cursor->get_columns();
            row_values *row = cursor->get_next_row();
            
            if (row) {
                int count = atoi(row->values[0]);
                if (count > 0) {
                    Serial.println("이미 등록된 ID입니다. 다른 ID를 골라주세요.");
                    delete cursor;
                    break;
                }
            }
            delete cursor;
            
            if (registerFingerprint()) {
                Serial.println("지문 등록이 완료되었습니다.");
            }
        }
        break;

      case 2: // 지문 삭제
        Serial.println("지문 삭제 프로세스 시작");
        Serial.println("1~127 중 지문 정보를 삭제하고 싶은 ID를 입력해주세요.");
        id = readnumber();
        deleteFingerprint(id);
        printTemplateCount();  
        break;

      default: // 잘못된 입력 처리
        Serial.println("잘못된 명령입니다. 1~2 사이의 번호를 입력하세요.");
        break;
    }
    Serial.println("\n1. 지문 등록, 2. 지문 삭제");
  }
}

// 지문 등록 함수
uint8_t registerFingerprint() {
  int p = -1;
  Serial.println("지문을 인식시켜주세요.");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("지문 인식 성공");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("통신 오류 발생");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("지문 인식 오류 발생");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(1);
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

  Serial.println("손가락을 떼어주세요.");
  delay(2000);
  p = 0;

  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  Serial.println("동일한 지문을 다시 인식시켜주세요.");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("지문 인식 성공");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("통신 오류 발생");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("지문 인식 오류 발생");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(2);
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

  Serial.print("지문 데이터를 확인합니다.\n");

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("지문 일치!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("통신 오류 발생");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("지문이 일치하지 않습니다.");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.print("지문이 등록되었습니다!");
    Serial.print(" (ID: "); Serial.print(id); Serial.print(")\n");

    if (mysqlConnected) {
      MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
      cursor->execute("USE sensor_data");

      String query = "INSERT INTO userinfo (fid) VALUES (" + String(id) + ")";

      
      if (cursor->execute(query.c_str())) {
        Serial.println("DB에 지문 ID가 등록되었습니다.");
      } else {
        Serial.println("DB 등록 중 오류가 발생했습니다.");
      }
      
      delete cursor;  
    } else {
      Serial.println("MySQL 연결이 되어있지 않습니다.");
    }

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("통신 오류 발생");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("지정한 위치에 저장이 불가능합니다.");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("등록 오류 발생");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  return true;
}

// 지문 삭제 함수
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.print("지문이 삭제되었습니다!");
    Serial.print(" (ID: "); Serial.print(id); Serial.print(")\n");
    
    if (mysqlConnected) {
      MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
      cursor->execute("USE sensor_data");

      String query = "DELETE FROM userinfo WHERE fid = " + String(id);
      
      if (cursor->execute(query.c_str())) {
        Serial.println("DB에서 지문 ID가 삭제되었습니다.");
      } else {
        Serial.println("DB 삭제 중 오류가 발생했습니다.");
      }
      
      delete cursor;  
    } else {
      Serial.println("MySQL 연결이 되어있지 않습니다.");
    }
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("통신 오류 발생");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("지정한 위치에 삭제가 불가능합니다.");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("삭제 오류 발생");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }
  return p;
}

// 지문 삭제 후 상태 확인 함수
void printTemplateCount() {
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.println("센서에 등록된 지문이 없습니다. 지문 등록을 먼저 진행해주세요!");
  } else {
    Serial.print("센서에 ");
    Serial.print(finger.templateCount);
    Serial.println("개의 지문이 저장되어 있습니다.");
  }
}