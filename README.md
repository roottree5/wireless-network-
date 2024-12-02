# 스마트 도어락 시스템 프로젝트_김연우

## 김연우 역할 및 작업 내용
---
**PWA 파트**

**목표**
웹/모바일 환경에서 동일하게 작동하는 스마트 도어락 제어 시스템 구현
실시간 출입 기록 모니터링 및 알림 시스템 구축
직관적인 사용자 인터페이스를 통한 도어락 원격 제어 구현

**과정**
React와 Node.js 기반의 PWA 환경 구축
MySQL 데이터베이스 연동으로 출입 기록 관리 시스템 구현
MQTT 프로토콜을 활용한 실시간 도어락 제어 및 알림 시스템 개발

**PWA 구조 소개**
저희 프로젝트는 웹 기반 애플리케이션으로, React로 구현한 프론트엔드와 Node.js로 구현한 백엔드로 구성했습니다.
프론트엔드에서는 사용자가 직접 보고 조작하는 인터페이스를, 백엔드에서는 데이터베이스 연동과 도어락 제어를 담당합니다.

**프론트엔드 주요 기능**

먼저 도어락 제어 화면입니다.

<img width="689" alt="doorlock_screen" src="https://github.com/user-attachments/assets/183df149-71a5-426b-a4d4-68b1cca7f7dd">

![mobile_doorlock_screen](https://github.com/user-attachments/assets/5b6cb92e-23b1-46ee-8bd5-c7967905c356)

- 중앙의 큰 버튼으로 도어락을 열고 닫을 수 있습니다
- 빨간색은 잠김, 초록색은 열림 상태를 나타냅니
- 버튼을 누르면 MQTT로 라즈베리파이에 제어 명령을 전송합니다

**출입 기록 화면**:
<img width="1492" alt="record" src="https://github.com/user-attachments/assets/910703e1-a3f5-46ee-acaf-8c5234abfcea">

![mobilerecord](https://github.com/user-attachments/assets/cdcf580b-b278-4485-9323-7585c7219326)

- RFID나 지문 인증 시도가 실시간으로 표시됩니다
- 성공한 경우 초록색, 실패한 경우 빨간색으로 구분됩니다
- 시간순으로 정렬되어 최신 기록을 바로 확인할 수 있습니다

**사용자 관리 화면**

<img width="1487" alt="user" src="https://github.com/user-attachments/assets/1c25161d-3127-4a2f-b655-3ddae2205d74">

![mobileuser](https://github.com/user-attachments/assets/62104978-fb12-4057-b97f-c32c90796fa7)


- 등록된 지문과 RFID 카드 목록을 확인할 수 있습니다

**백엔드 동작 과정**
1.데이터베이스 연결 설정
const pool = mysql.createPool({
  host: process.env.DB_HOST || '서버컴퓨터IP',
  user: process.env.DB_USER || 'test',
  password: process.env.DB_PASSWORD || '12345678',
  database: process.env.DB_NAME || 'sensor_data'
});

2.Node.js(백엔드 서버 API 엔드 포인트 , Restful api 구현)
router.get('/fingerprint-logs', async (_req, res) => {
  try {
    const [rows] = await pool.query(`
      SELECT 
        id,
        COALESCE(fingerp_confirm_time, fingerp_event) as confirm,
        door_status,
        CASE 
          WHEN fingerp_confirm_time IS NOT NULL THEN 1
          ELSE 0 
        END as success,
        'fingerprint' as type
      FROM event_manage 
      WHERE fingerp_confirm_time IS NOT NULL 
         OR fingerp_event IS NOT NULL
    `);
    res.json({ success: true, data: rows });
  } catch (error) {
    res.status(500).json({ success: false, message: '지문인식 로그 조회 실패' });
  }
});

3. 프론트엔드의 API 호출
getFingerprintLogs: async (): Promise<ApiResponse<AccessLog[]>> => {
  try {
    const response = await doorlockApi.fetchWithTimeout<ApiResponse<AccessLog[]>>(
      `${API_BASE_URL}/access-logs/fingerprint-logs`
    );
    return response;
  } catch (error) {
    console.error('지문 로그 조회 중 오류 발생:', error);
    throw error;
  }
}

**전체 데이터 흐름**
3-1 프론트엔드(React)에서 API 요청 발생
3-2 백엔드 서버가 요청을 받아 MySQL 쿼리 실행
3-3 데이터베이스에서 결과를 받아 JSON 형태로 변환
3-4 프론트엔드로 응답을 전송
3-5 프론트엔드에서 데이터를 화면에 표시

**출입 기록은 5초마다, 도어 상태는 1초마다 자동으로 갱신되어 실시간으로 정보를 제공합니다.**

4. MQTT 통신 구조 (1분)
도어락 제어 명령은 다음 순서로 전달됨:
// DoorControl.tsx의 MQTT 통신 부분
const handleToggleDoor = async () => {
  try {
    setIsLoading(true);
    const command = isLocked ? 'OPEN' : 'CLOSE';
    const timestamp = new Date().toISOString();

    // MQTT로 도어락 제어 명령 전송
    mqttClient.publish('doorlock/control', JSON.stringify({
      command,
      timestamp,
      status: isLocked ? 1 : 0
    }));

    // 서버에 상태 업데이트
    await doorlockApi.updateDoorStatus(isLocked ? 1 : 0);

    console.log('도어 제어 명령 전송:', command);
    setIsLocked(!isLocked);
  } catch (error) {
    console.error('도어락 제어 실패:', error);
    toast.error('도어락 제어에 실패했습니다.');
  }
};

// server/src/index.ts의 MQTT 서버 설정
mqttClient.on('message', async (topic, message) => {
  try {
    const data = JSON.parse(message.toString());
    if (topic === 'doorlock/control') {
      // 도어락 제어 명령 처리
      await pool.execute(`
        INSERT INTO event_manage 
        (door_status, pwa_confirm_time)
        VALUES (?, ?)
      `, [data.status, data.timestamp]);
    }
  } catch (error) {
    console.error('MQTT 메시지 처리 실패:', error);
  }
});
- PWA(프론트엔드)에서 도어락 제어 버튼 클릭 or 터치 
- MQTT 프로토콜을 통해 라즈베리파이로 명령이 전송
- 라즈베리파이는 이 명령을 아두이노로 전달.
- 동시에 MySQL에 상태 변경 기록
- 프론트엔드에서 상태 변경 표시
- 실제 도어락이 동작하게 됩니다

5. 알림 시스템
   
<img width="1499" alt="pcalarm" src="https://github.com/user-attachments/assets/40d1e68b-a0d9-454c-a661-cbce40c876c4">

![mobilealarm](https://github.com/user-attachments/assets/49c8ece6-ea81-43ef-a307-caa975938a65)

**알림 시스템 동작 과정**

**알림 발생 조건과 처리**

// 미등록 지문/RFID 시도 감지
const getStatusMessage = (log: NotificationLog, type: 'fingerprint' | 'rfid') => {
 if (log.door_status === 1) {
   return `${type === 'fingerprint' ? '지문' : 'RFID 카드'}로 정상 출입했습니다.`;
 } else {
   return `${type === 'fingerprint' ? '미등록 지문' : '미등록 RFID 카드'}으로 출입 시도가 있었습니다.`;
 }
};

// 새로운 알림 확인 및 브라우저 알림 생성
const latestNotification = existingNotifications[0];
if (latestNotification && !latestNotification.read && Notification.permission === 'granted') {
 new Notification(latestNotification.title, {
   body: latestNotification.message,
   icon: '/alert-icon.png'
 });
}

MQTT를 통한 알림 전달
// 미등록 접근 시도 감지 시
mqttClient.publish('doorlock/notification', JSON.stringify({
  type: 'unauthorized_access',
  message: `미등록 ${type === 'fingerprint' ? '지문' : 'RFID 카드'} 접근 시도`,
  timestamp: new Date().toISOString()
}));

**알림 발생 흐름** 
미등록 출입 시도 발생
서버에서 이벤트 감지
MQTT로 알림 메시지 전송
프론트엔드에서 알림 수신 및 표시
브라우저/모바일 푸시 알림 생성

**주요 알림 종류**

미등록 지문/RFID 시도
도어락 상태 변경
인증 성공/실패

이렇게 실시간으로 모든 이벤트를 사용자에게 알려주어 보안성을 강화했습니다.

**이 시스템을 통해 웹과 모바일에서 동일하게 실시간 알림을 받을 수 있으며, 미등록 접근 시도나 도어락 상태 변경 등 중요한 이벤트를 즉시 확인할 수 있습니다.**
