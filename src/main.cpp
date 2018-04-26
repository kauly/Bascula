#include <SX1272.h>
#include <Wire.h>
#include "RTClib.h"
#include <time.h>
#include <sys/time.h>

#define MAX_DBM 14
#define LORAMODE  2
#define node_addr 8
#define DEFAULT_DEST_ADDR 1

const uint32_t DEFAULT_CHANNEL=CH_00_900;

int loraMode=LORAMODE;

const int reed_pin = 34;
//Capacidade da bascula em mm
const float cap_basc = 0.25;
//Em caso de emergencia enviar os dados imediatamente 
const int emergencia_min = 2;
const int emergencia_basc = 12;



int reed_monitor = 0;
int reed_count = 0;
float chuva_mm = 0;

float dados_arr[24];

RTC_DS3231 rtc;
time_t timer;
struct tm * tempo_atual;
int tempo_inicial = 0;

void pop_arr(float num, float * arr_p){
    for(int i=0; i<24; i++){
        arr_p[i] = num;
    }
}

String arr_to_str(float * arr_p){
    String str = String("\\!");
    for (int i = 0;i < 24;i++){
        String valor = String(arr_p[i]);
        //delimitador ">"
        if(i < 23)
          valor.concat(">");
        str.concat(valor);
    }
    return str;
}




void conf_tempo(){
  int ctr = 0;
  DateTime now = rtc.now();
  struct timeval tempo;
  tempo.tv_sec  = now.unixtime();
  tempo.tv_usec = 0;
  if(settimeofday(&tempo, NULL) == 0){
    Serial.print("Sucesso");
  }else {
    Serial.println("Erro");
  }
  delay(500);
}



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

void envia_dado(String dados_p){  
  uint8_t r_size = dados_p.length()+1;
  uint8_t message[r_size];
  dados_p.toCharArray((char *)message, r_size);
  int e;
  sx1272.CarrierSense();
  sx1272.setPacketType(PKT_TYPE_DATA);
  e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, r_size);
  Serial.print("Dado enviado, estado: ");
  Serial.println(e);
 // delay(1000*1000);
  delay(1000);
}

void ver_emergencia(int atual, int inicial, int count){

  int delta_tempo = atual - inicial;
  if(delta_tempo <= emergencia_min && count >= emergencia_basc){
    Serial.println("EMERGENCIA");
    Serial.print("Atual: ");
    Serial.print(atual);
    Serial.print("Inicial: ");
    Serial.print(inicial);
    tempo_inicial = atual;
    envia_dado("66");
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("COM Serial iniciada");
  pinMode(reed_pin, INPUT);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  conf_tempo();
  conf_radio();
  pop_arr(14.89, dados_arr);
}

void loop() {
  
  time(&timer);
  tempo_atual = localtime(&timer);
  reed_monitor = digitalRead(reed_pin);
  if (reed_monitor != 0){
    delay(400);
    reed_count++;
    reed_monitor = 0;
    if(reed_count == 1)
      tempo_inicial = tempo_atual->tm_min;
    dados_arr[tempo_atual->tm_hour] = dados_arr[tempo_atual->tm_hour] + 1;
    Serial.println("Girou");
    Serial.println(reed_count);  
    ver_emergencia(tempo_atual->tm_min, tempo_inicial, reed_count);

  }
  if(tempo_atual->tm_hour == 0 && tempo_atual->tm_min == 0){
    envia_dado(arr_to_str(dados_arr));
    pop_arr(0, dados_arr);
    reed_count = 0;
  }
  if(reed_count == 4){
    envia_dado(arr_to_str(dados_arr));
    reed_count = 0;
  }
  
 
}