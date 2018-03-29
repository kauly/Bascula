#include <SX1272.h>
#include <Wire.h>
#include <RTClib.h>

#define MAX_DBM 14
#define LORAMODE  2
#define node_addr 8
#define DEFAULT_DEST_ADDR 1

const uint32_t DEFAULT_CHANNEL=CH_05_900;

int loraMode=LORAMODE;

const int reed_pin = 12;
//Capacidade da bascula em mm
const float cap_basc = 0.25;
//Em caso de emergencia enviar os dados imediatamente 
const int emergencia = 20;
//Identificacao do pluviometro


int reed_monitor = 0;
int reed_count = 0;
float chuva_mm = 0;

RTC_DS3231 rtc;

void conf_radio(){
  int e;  
  sx1272.setCSPin(18);
  sx1272.ON();
  e = sx1272.setMode(loraMode);
  Serial.print("Radio iniciado MODO: ");
  Serial.println(e);
  // enable carrier sense
  sx1272._enableCarrierSense=true;  
  // Select frequency channel
  e = sx1272.setChannel(DEFAULT_CHANNEL);
  Serial.print("Canal: ");
  Serial.println(e);
  sx1272._needPABOOST=true;
  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
  Serial.print("Max DBM: ");
  Serial.println(e);
  // Set the node address and print the result
  e = sx1272.setNodeAddress(node_addr);
  Serial.print("NODE ADDR: ");
  Serial.println(e);
  Serial.println("Fim da conf do radio");
  delay(500);
}

void envia_dado(){
  uint8_t message[30];
  uint8_t r_size;
  int e;
  sx1272.CarrierSense();
  sx1272.setPacketType(PKT_TYPE_DATA);
  r_size=sprintf((char*)message, "%f", reed_count*cap_basc);
  e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, r_size);
  Serial.print("Dado enviado, estado: ");
  Serial.println(e);
  reed_count = 0;
  delay(500);
}

void setup() {
  Serial.begin(9600);
  Serial.println("COM Serial iniciada");
  pinMode(reed_pin, INPUT);
  conf_radio();
  rtc.begin();
  
  
}

void loop() {
  
//DateTime now = rtc.now(); 
  reed_monitor = digitalRead(reed_pin);
  
  if (reed_monitor != 0){
    delay(200);
    reed_count++;
    reed_monitor = 0;
    Serial.println("Girou");
    Serial.println(reed_count);  
  }

  if (reed_count == 4 )
    envia_dado();
}