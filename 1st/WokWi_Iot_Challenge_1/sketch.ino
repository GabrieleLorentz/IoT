#include <WiFi.h>
#include <esp_now.h>
//Defining pin for sensor connection
#define TRIG_PIN 16
#define ECHO_PIN 18

//minimum distance to consider free/occupied the parking 
#define MINUM_DISTANCE 50

// MAC receiver
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xFB, 0x90};

const int SLEEP_TIME = 34000000; //since my personal code is 10774229, the sleep tipe has been calculated as (34 % 50) + 5, already multiplied to obtain the value in microseconds
esp_now_peer_info_t peerInfo;

// Sending and receiving callback 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent" : "Error");
}


void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
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
  uint16_t distance = duration / 58; // distanza in cm

  if (distance <= MINUM_DISTANCE) {
    return "OCCUPIED";
  } else {
    return "FREE";
  }
}

void Setup_Wifi(){
  //Turning on wifi
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm); //setting the transmission power to 2dBm

  //Registration of callback
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  //registration of peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

}

void setup() {
  unsigned long Start = micros(); //Starting time registered for reference

  Serial.begin(115200); //initialiting board and pins

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.print("* BOOT TIME, ");
  Serial.println(micros() - Start); //registering boot time (considered as idle time -> phase where the board is turned on but nothing is appening)

  // Calling the function to calculate the distance with sensor HC-SR04
  String msg = getSensorMessage();

  Serial.print("* MISURATION, ");
  Serial.println(micros() - Start); //registering misuration time
  
  Setup_Wifi(); //calling the function to turning on the Wi-Fi

  Serial.print("* WIFI TURNED ON, ");
  Serial.println(micros() - Start); //registered time when wifi has been turned on

  // Transmitting using ESP-NOW protocol
  esp_now_send(broadcastAddress, (uint8_t *)msg.c_str(), msg.length());

  delay(1000); //Delay to see the message received considering that the sink node is the board itself
  
  Serial.print("* MESSAGE SENT, ");
  Serial.println(micros() - Start); //Registered time when the message has been delivered

  WiFi.mode(WIFI_OFF); //turning off wifi

  Serial.print("* WIFI TURNED OFF, ");
  Serial.println(micros() - Start); //registered time when wifi has been turned off, this time is also the time usefull to understand when the board is going to sleep

  // deep-sleep per 34 secondi
  esp_sleep_enable_timer_wakeup(SLEEP_TIME); //setting deep-sleep time
  esp_deep_sleep_start(); //going to sleep
}

void loop() {
  // Since the deep-sleep is called in setup function, the loop function is never called
}
