#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

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




#define RST_PIN D3  // Reset 핀
#define SS_PIN D2   // Chip Select 핀

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID 인스턴스 생성

void setup() {
  Serial.begin(115200);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

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

  Serial.println("카드 관리 시스템 시작");
  Serial.println("\n1. 카드 등록, 2. 카드 삭제");
}

uint8_t readnumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  // Wi-Fi 연결 확인
  

  if (Serial.available()) {
    int command = readnumber();  // 입력값 읽기
    switch (command) {
      case 1:
        Serial.println("카드 등록 프로세스 시작");
        Serial.println("등록하실 카드를 RFID 센서에 인식시켜주세요.");
        registerCard();
        break;

      case 2:
        Serial.println("카드 삭제 프로세스 시작");
        Serial.println("삭제하실 카드를 RFID 센서에 인식시켜주세요.");
        deleteCard();
        break;

      default:
        Serial.println("잘못된 명령입니다. 1~2 사이의 번호를 입력하세요.");
        break;
    }
    Serial.println("\n1. 카드 등록, 2. 카드 삭제");
  }
}

// 카드 등록 함수
void registerCard() {
  String tag_uid = "";
  while (tag_uid == "") {
    tag_uid = checkRFID();  
    delay(100);
  }

  Serial.print("등록할 카드 UID: ");
  Serial.println(tag_uid);

  if (mysqlConnected) {
    MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
    cursor->execute("USE sensor_data");

    // 카드 존재 확인 쿼리
    String checkQuery = "SELECT COUNT(*) AS count FROM userinfo WHERE uid = '" + tag_uid + "'";
    cursor->execute(checkQuery.c_str());
    
    // 결과 확인
    column_names *columns = cursor->get_columns();
    row_values *row = cursor->get_next_row();
    
    if (row) {
      int count = atoi(row->values[0]);
      
      if (count > 0) {
        Serial.println("이미 등록된 카드입니다. 중복 등록을 방지합니다.");
        delete cursor;
        return;
      }
    }

    // 카드 등록 쿼리
    String query = "INSERT INTO userinfo (uid) VALUES ('" + tag_uid + "')";
    
    if (cursor->execute(query.c_str())) {
      Serial.println("카드 등록 완료.");
    } else {
      Serial.println("카드 등록 중 오류가 발생했습니다.");
    }
    
    delete cursor;  
  } else {
    Serial.println("MySQL 연결이 되어있지 않습니다.");
  }
}

// 카드 삭제 함수
void deleteCard() {
  String tag_uid = "";
  while (tag_uid == "") {
    tag_uid = checkRFID();  
    delay(100);
  }

  Serial.print("삭제할 카드 UID: ");
  Serial.println(tag_uid);

  if (mysqlConnected) {
    MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
    cursor->execute("USE sensor_data");

    // 카드 존재 확인 쿼리
    String checkQuery = "SELECT COUNT(*) AS count FROM userinfo WHERE uid = '" + tag_uid + "'";
    cursor->execute(checkQuery.c_str());
    
    // 결과 확인
    column_names *columns = cursor->get_columns();
    row_values *row = cursor->get_next_row();
    
    if (row) {
      int count = atoi(row->values[0]);
      
      if (count == 0) {
        Serial.println("등록되지 않은 카드입니다. 삭제할 수 없습니다.");
        delete cursor;
        return;
      }
    }

    // 카드 삭제 쿼리
    String query = "DELETE FROM userinfo WHERE uid = '" + tag_uid + "'";
    
    if (cursor->execute(query.c_str())) {
      Serial.println("카드 삭제 완료.");
    } else {
      Serial.println("카드 삭제 중 오류가 발생했습니다.");
    }
    
    delete cursor;  
  } else {
    Serial.println("MySQL 연결이 되어있지 않습니다.");
  }
}
// 카드 UID 반환
String checkRFID() {
  String tag_uid = "";
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return tag_uid;
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tag_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tag_uid.toUpperCase();
  return tag_uid;
}
