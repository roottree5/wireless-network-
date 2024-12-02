import { useState, useEffect } from 'react'; 
import mqtt from 'mqtt';


function App() {
//클라이언트 초기화
  const [payload, setPayload] = useState({ topic: "", message: "" });
  const [connectStatus, setConnectStatus] = useState("Disconnected");
  const clientId = 'react_client';
  const [client, setClient] = useState(null);

//라즈베리 ip mosquitto 포트 1883에 연결
  useEffect(() => {
    const mqttClient = mqtt.connect("192.168.0.42:1883", {
      clientId,
    });

    setClient(mqttClient);

//커넥션 핸들러
    const handleConnect = () => {
      setConnectStatus("Connected");

// 테스트를 위한 Topic subscribe
      mqttClient.subscribe("connectTest", (err) => {
        if (!err) {
          console.log("Subscribed to Testconnection"); 
        }
      });
    };

//에러 핸들러
    const handleError = (err) => {
      console.error("Connection error: ", err);
      setConnectStatus("Disconnected");
      mqttClient.end();
    };

//재접속 핸들러
    const handleReconnect = () => {
      setConnectStatus("Reconnecting");
    };

//메시지 관리
    const handleMessage = (topic, message) => {
      setPayload({ topic, message: message.toString() });
      console.log(message.toString());
    };

    mqttClient.on("connect", handleConnect);
    mqttClient.on("error", handleError);
    mqttClient.on("reconnect", handleReconnect);
    mqttClient.on("message", handleMessage);

    return () => {
      mqttClient.off("connect", handleConnect);
      mqttClient.off("error", handleError);
      mqttClient.off("reconnect", handleReconnect);
      mqttClient.off("message", handleMessage);

      mqttClient.end();
    };
  }, []);

//클릭이벤트나 문열어야 할때 호출할 함수
  const handlePublish = () => {
    if (client) {
      client.publish("fRasberry", "Switch_door_status");
    }
  };

}