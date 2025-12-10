#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define RCWL_PIN 25
#define PIN        26
#define NUMPIXELS 16

#define RECEIVING 0
#define SENDING 1

/* === Function declaration === */
void handle_motion_detected ();
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status);
void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t *incomingData, int len);
void transmit();
void setNeo();

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

/* === Global variables === */
volatile int state_flag = RECEIVING;
uint8_t broadcastAddress[] = {0xA8, 0x03, 0x2A, 0xEA, 0xE8, 0x54};

typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

/* @OnDataSent
 *
 * ESPNOW transmission callback function
 *
 * */

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {


    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/* @OnDataRecv
 *
 * ESPNOW Receiver's callback function
 *
 * */

void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t *incomingData, int len){
  setNeo();

  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("Bool: ");
  Serial.println(myData.d);
  Serial.println();

}

void setup() {

  Serial.begin(115200);

  /* pixel configuration */
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin();

  /* set interrupt */
  attachInterrupt(RCWL_PIN, my_callback, RISING);

  /* Set device as a Wi-Fi Station */
  WiFi.mode(WIFI_STA);

  /* Init ESP-NOW */
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  /* Initialize transmission callback */
  esp_now_register_send_cb(OnDataSent);

  /* Initialize receiver's callback */
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  /*configure RCWL PIN*/
  pinMode(RCWL_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  state_flag = RECEIVING;

}

/* Interrupt callback*/
void ARDUINO_ISR_ATTR my_callback() {
  state_flag = SENDING;
}

/* @handle_motion_detected
 *
 * Transmission logic: fill packet fields & call esp_now_send() to transmit packet
 * */

void transmit(){
  // Set values to send
  strcpy(myData.a, "Motion detected");
  myData.b = random(1,20);
  myData.c = 1.2;
  myData.d = false;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

}

/* @handle_motion_detected
 *
 * Turn on ESP32 BUILTIN LED, reset state_flag to RECEVING & call transmission()
 *
 * */

void handle_motion_detected (){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
  transmit();
  state_flag = RECEIVING; // go back to receiving state
}


/* @setNeo
 *
 * Turn all neopixels RED upon receiving
 *
 * */

void setNeo(){
  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }

  pixels.show();
  delay(6000);
  pixels.clear();
  pixels.show();
}

void loop() {
  if (state_flag == SENDING){
    handle_motion_detected();
  }
}
