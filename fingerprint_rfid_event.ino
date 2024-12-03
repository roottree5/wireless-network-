#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <PubSubClient.h>

#define Relay D8  // 릴레이를 연결한 핀


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(D4, D1);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 mfrc522(D2, D3);  // SS_PIN, RST_PIN 설정


// Wi-Fi 정보
const char* ssid = "서준원의 iphone";
const char* password = "11111111";

// MySQL 서버 정보
IPAddress server_addr(10, 200, 43, 106);  // MySQL 서버 IP 주소.    10.200.43.106
char user[] = "test";                    // MySQL 사용자명
char pass[] = "89352183";                // MySQL 비밀번호


// mqtt 서버 정보
const char* mqtt_server = "192.168.0.42";


// MySQL 연결 객체 생성
WiFiClient mysql_client;
MySQL_Connection conn(&mysql_client);

//mqtt통신 연결 객체 생성
WiFiClient MQTT_client;
PubSubClient mqtt_client(MQTT_client);
char msg[50];
int value = 0;
String receivedMessage = ""; // 수신된 메시지를 저장하는 변수



// 상태 플래그
bool wifiConnected = false;
bool mysqlConnected = false;

// 변수 초기화
int fingerFailCounter = 0;  // 지문 인식 실패 횟수 카운터
int RFIDFailCounter = 0;    // RFID 탐색 실패 횟수 카운터



unsigned long previousMillis = 0;  // 마지막 시간
const long interval = 7000;  // 10초 (10000ms)



int door_status = 0;
int currentState = 0;


  
void setup() {
  Serial.begin(115200);
  while (!Serial);  
  delay(100);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  // WIFI 연결 확인
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


  //mqtt 연결
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  
  //지문센서 연결 
  finger.begin(57600);
  delay(100);
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
  Serial.println("RFID, 지문인식을 시작합니다");

  pinMode(Relay, OUTPUT); // 릴레이 핀 출력으로 설정
  digitalWrite(Relay, LOW); // 초기 릴레이 OFF
}

void loop() {
  // 지문 인식 처리
  if (getFingerprintID() == FINGERPRINT_NOFINGER) {
    // RFID 인식 처리
    checkRFID();
  }
  Serial.println("RFID, 지문인식을 대기중입니다.");
  delay(50);
  


  // 문이 열려 있는 상태에서 n초 후에 문을 닫기
    if (door_status == 1) {
      unsigned long currentMillis = millis();  // 현재 시간

       if (currentMillis - previousMillis >= interval) {
        Serial.println("문 닫기");

        currentState = digitalRead(Relay);  

        digitalWrite(Relay, !currentState);
        delay(100);  // 약간의 지연 후 선을 다시 연결 (문 닫힘)
        digitalWrite(Relay, currentState);
        
        door_status = 0;  // 문 닫힘
        previousMillis = 0;  // 시간을 초기화하여 다음 열림 동작을 위해 새로 시작

        //DB에 문상태 올리자
        if (mysqlConnected) {
          MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
          cursor->execute("USE sensor_data");




          // event와 door_status를 모두 포함한 INSERT 쿼리 작성
          String query = "INSERT INTO event_manage (door_status) VALUES (" + String(door_status) + ")";

          if (cursor->execute(query.c_str())) {
              Serial.println("데이터 삽입 완료.");
          } else {
              Serial.println("데이터 삽입 중 오류가 발생했습니다.");
          }

          delete cursor;  
        } else {
            Serial.println("MySQL 연결이 되어있지 않습니다.");
        }

      }
    }

  //   if (!mqtt_client.connected()) {
  //   reconnect();
  // } 
  // mqtt_client.loop();//mqtt통신 들어온게 있나 확인



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
  delay(1000);
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
    if (door_status == 0) {
            Serial.println("문 열기");
            currentState = digitalRead(Relay);
            digitalWrite(Relay, !currentState);  // 릴레이 ON (문 열림)
            delay(500);
            digitalWrite(Relay, currentState);
            door_status = 1;  // 문 상태 변경
            previousMillis = millis();  // 현재 시간 기록
          }
    //여기서 지문이 일치되어서 문열린 시간 DB에 추가


    if (mysqlConnected) {
          MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
          cursor->execute("USE sensor_data");

          // door_status 값을 숫자로 설정 (예: 1 = 열림, 0 = 닫힘)
          door_status = 1; // 문제가 발생하지 않았으니까 default는 열림으로 설정해서 컬럼 추가할께요

          // event와 door_status를 모두 포함한 INSERT 쿼리 작성
          String query = "INSERT INTO event_manage (fingerp_confirm_time, door_status) VALUES (NOW(), " + String(door_status) + ")"; //지문으로 인한 문열림 발생시간(현시간)을 timestamp형태로 추가합니다

          if (cursor->execute(query.c_str())) {
              Serial.println("데이터 삽입 완료.");
          } else {
              Serial.println("데이터 삽입 중 오류가 발생했습니다.");
          }

          delete cursor;  
        } else {
            Serial.println("MySQL 연결이 되어있지 않습니다.");
        }


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
      //여기서 DB에 지문탐색 이벤트 발생 컬럼에 추가해준다

      if (mysqlConnected) {
          MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
          cursor->execute("USE sensor_data");

          // door_status 값을 숫자로 설정 (예: 1 = 열림, 0 = 닫힘)
          door_status = 0; // 문제가 발생했으니까 default는 닫힘으로 설정해서 컬럼 추가할께요

          // event와 door_status를 모두 포함한 INSERT 쿼리 작성
          String query = "INSERT INTO event_manage (fingerp_event, door_status) VALUES (NOW(), " + String(door_status) + ")"; //event발생시간(현시간)을 timestamp형태로 추가합니다

          if (cursor->execute(query.c_str())) {
              Serial.println("데이터 삽입 완료.");
          } else {
              Serial.println("데이터 삽입 중 오류가 발생했습니다.");
          }

          delete cursor;  
        } else {
            Serial.println("MySQL 연결이 되어있지 않습니다.");
        }


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
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    tag_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  tag_uid.toUpperCase();
  Serial.println(tag_uid);

  // MySQL을 사용하여 등록된 카드인지 확인
  bool isRegistered = false;

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
      if (count > 0) { //count값이 양수라는건 일치하는 id값이 있다는 얘기
        isRegistered = true;
      }
    }
    delete cursor;
  } else {
    Serial.println("MySQL 연결이 되어있지 않습니다.");
  }

  // 등록 여부에 따른 동작
  if (isRegistered) {
    Serial.println("등록된 카드입니다.");

    // 등록된 카드 인식 시 이벤트 추가
    if (door_status == 0) {
            Serial.println("문 열기");
            currentState = digitalRead(Relay);
            digitalWrite(Relay, !currentState);  // 릴레이 ON (문 열림)
            delay(500);
            digitalWrite(Relay, currentState);
            door_status = 1;  // 문 상태 변경
            previousMillis = millis();  // 현재 시간 기록
          }

    if (mysqlConnected) {
          MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
          cursor->execute("USE sensor_data");

          // door_status 값을 숫자로 설정 (예: 1 = 열림, 0 = 닫힘)
          door_status = 1; // 문제가 발생하지 않았으니까 default는 열림으로 설정해서 컬럼 추가할께요

          // event와 door_status를 모두 포함한 INSERT 쿼리 작성
          String query = "INSERT INTO event_manage (rfid_confirm_time, door_status) VALUES (NOW(), " + String(door_status) + ")"; //지문으로 인한 문열림 발생시간(현시간)을 timestamp형태로 추가합니다

          if (cursor->execute(query.c_str())) {
              Serial.println("데이터 삽입 완료.");
          } else {
              Serial.println("데이터 삽입 중 오류가 발생했습니다.");
          }

          delete cursor;  
        } else {
            Serial.println("MySQL 연결이 되어있지 않습니다.");
        }



    RFIDFailCounter = 0;  // UID 일치 시 실패 카운터 초기화
  } else {
    Serial.println("등록되지 않은 카드입니다.");
    RFIDFailCounter++;  // 실패 시 카운터 증가
    if (RFIDFailCounter >= 3) {  // 실패가 3회 이상이면 이벤트 발생
      Serial.println("RFID 탐색 실패 3회 발생 - 추가 이벤트 실행!");
      // 여기서 RFID 이벤트 발생 컬럼 추가한다
      // 추가적인 이벤트 코드를 여기에 작성

      if (mysqlConnected) {
          MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
          cursor->execute("USE sensor_data");

          // door_status 값을 숫자로 설정 (예: 1 = 열림, 0 = 닫힘)
          door_status = 0; // 문제가 발생했으니까 default는 닫힘으로 설정해서 컬럼 추가할께요

          // event와 door_status를 모두 포함한 INSERT 쿼리 작성
          String query = "INSERT INTO event_manage (rfid_event, door_status) VALUES (NOW(), " + String(door_status) + ")"; //event발생시간(현시간)을 timestamp형태로 추가합니다

          if (cursor->execute(query.c_str())) {
              Serial.println("데이터 삽입 완료.");
          } else {
              Serial.println("데이터 삽입 중 오류가 발생했습니다.");
          }

          delete cursor;  
        } else {
            Serial.println("MySQL 연결이 되어있지 않습니다.");
        }

      
      RFIDFailCounter = 0;  // 카운터 초기화
    }
  }

  delay(3000);
  Serial.println("RFID 인식 대기 중");
}


void reconnect() {
  while (!mqtt_client.connected()) {
    Serial.print("Wait for MQTT connection...");
    if (mqtt_client.connect("ESP8266Client")) {
      Serial.println("connected");
      //publishing MQTT topic , message
      mqtt_client.publish("fArduino", "데이터값 필요");
      //subscribing topic
      mqtt_client.subscribe("fRasberry");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//when subscribing topic send message
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // 수신된 payload를 String으로 변환 후 저장
  receivedMessage = ""; // 기존 데이터 초기화
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }

  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  Serial.print(receivedMessage);
  Serial.println();

  //리시브 메시지를 받았을때 이코드가 실행되기 때문에 문상태를 변경하겠습니다.
  if (door_status == 0) {
            Serial.println("문 열기");
            currentState = digitalRead(Relay);
            digitalWrite(Relay, !currentState);  // 릴레이 ON (문 열림)
            delay(500);
            digitalWrite(Relay, currentState);
            door_status = 1;  // 문 상태 변경
            previousMillis = millis();  // 현재 시간 기록


            if (mysqlConnected) {
              MySQL_Cursor* cursor = new MySQL_Cursor(&conn);
              cursor->execute("USE sensor_data");


              // event와 door_status를 모두 포함한 INSERT 쿼리 작성
              String query = "INSERT INTO event_manage (pwa_confirm_time, door_status) VALUES (NOW(), " + String(door_status) + ")"; //지문으로 인한 문열림 발생시간(현시간)을 timestamp형태로 추가합니다

              if (cursor->execute(query.c_str())) {
                  Serial.println("데이터 삽입 완료.");
              } else {
                  Serial.println("데이터 삽입 중 오류가 발생했습니다.");
              }

              delete cursor;  
            } else {
                Serial.println("MySQL 연결이 되어있지 않습니다.");
              }

       
          }
    

}



