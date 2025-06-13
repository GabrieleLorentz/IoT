#include <WiFi.h>
#include <esp_now.h>

#define TRIG_PIN 16
#define ECHO_PIN 18

#define MINUM_DISTANCE 50

// MAC receiver
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xFB, 0x90};

const int SLEEP_TIME = 34000000; // ....... => since my personal code is 10774229, 34 has been obtained by the formula: 29 % 50 + 5 = 34
unsigned long StartingTime;
esp_now_peer_info_t peerInfo;


// Sending and receiving callback 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent" : "Error");

}

void OnDataRecv(const uint8_t * mac, const uint8_t *data, int len) {
  Serial.print("Sink Node received: ");
  char receivedString[len + 1];
  memcpy(receivedString, data, len);
  receivedString[len] = '\0';

  Serial.println(String(receivedString)); 
}


String getSensorMessage() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
  
    uint16_t duration = pulseIn(ECHO_PIN, HIGH);
    uint16_t distance = duration / 58; // distance in cm

    if (distance <= MINUM_DISTANCE) {
        return "OCCUPIED";
    } else {
        return "FREE";
    }
}

void Setup_Wifi(){
WiFi.mode(WIFI_STA); // Turning on wifi

WiFi.setTxPower(WIFI_POWER_2dBm); //Setting wifi power


esp_now_init();


//callback
esp_now_register_send_cb(OnDataSent);
esp_now_register_recv_cb(OnDataRecv); //Added for debug to see if the connection works

// Peer Registration
memcpy(peerInfo.peer_addr, broadcastAddress, 6);
peerInfo.channel = 0;  
peerInfo.encrypt = false;
//Adding a peer
esp_now_add_peer(&peerInfo);
}

void setup() {

    unsigned long Start = micros();
    //Initial setup
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.print("* BOOT TIME, ");
    Serial.println(micros()-Start);

    // Calculating distance with the sensor (HC-SR04)
    String msg = getSensorMessage();

    // Calculating the time required for the simulation
    Serial.print("* MISURATION, ");
    Serial.println(micros() - Start);
    
    //starting wifi communication

    Setup_Wifi();

    Serial.print("* WIFI TURNED ON, ");
    Serial.println(micros()-Start);

    // Sending the message using ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)msg.c_str(), msg.length());

    delay(1000);
    
    Serial.print("* MESSAGE SENT, ");
    Serial.println(micros() - Start);


    WiFi.mode(WIFI_OFF);

    Serial.print("* WIFI TURNED OFF, ");
    Serial.println(micros()-StartingTime);



    // deep-sleep for 34 sec
    esp_sleep_enable_timer_wakeup(SLEEP_TIME); 
    esp_deep_sleep_start();
}

void loop() {
  //As the deep-sleep has been implemented in code, the loop is never executed
}
