#include "HardwareSerial.h"

#ifndef _FINGERPRINTER_H_
#define _FINGERPRINTER_H_

//指纹串口对应io口设置
#define UART_RX_PIN 17
#define UART_TX_PIN 18
//Serial1 17TX,18RX

const uint32_t communicationTimeout_ms = 100;
uint8_t RX_flag=0;

String Serial0_buffer;//debug
String Serial1_buffer;//fingerprint

SemaphoreHandle_t Serial0_buffer_Mutex = NULL;//debug
SemaphoreHandle_t Serial1_buffer_Mutex = NULL;//fingerprint

uint8_t FingerPrint_CMD[7][8] = {
  {0xF5,0x26,0x00,0x00,0x00,0x00,0x26,0xF5},//version*
  {0xF5,0x01,0x00,0x00,0x01,0x00,0x00,0xF5},//reg*
  {0xF5,0x05,0x00,0x00,0x00,0x00,0x05,0xF5},//delete*
  {0xF5,0x0D,0x00,0x00,0x00,0x00,0x0D,0xF5},//ID*
  {0xF5,0x0C,0x00,0x00,0x00,0x00,0x0C,0xF5},//match*
  {0xF5,0x09,0x00,0x00,0x00,0x00,0x09,0xF5},//count*
  {0xF5,0x24,0x00,0x00,0x00,0x00,0x24,0xF5},//image
};

void fg_getVersion(void);
uint8_t fg_fingerReg(void);
void fg_deteleFinger(void);
uint8_t fg_getFreeID(void);
uint8_t fg_matchFinger(void);
uint8_t fg_getNumber(void);
void fg_getImage(void);

//调试串口0中断
void Serial0_interruption(void){
  uint8_t get_number=0;
  Serial0_buffer.clear();
  if (xSemaphoreTake(Serial0_buffer_Mutex, portMAX_DELAY)) {
    uint32_t now = millis();
    while ((millis() - now) < communicationTimeout_ms) {
      if (Serial0.available()) {
        Serial0_buffer += (char)Serial0.read();
        now = millis();
      }
    }
    xSemaphoreGive(Serial0_buffer_Mutex);
    Serial0.println(Serial0_buffer);
    if(Serial0_buffer=="ver"){
      fg_getVersion();
    }else if(Serial0_buffer=="reg"){
      get_number = fg_fingerReg();
      Serial0.print("reg id:");
      Serial0.println(get_number);
    }else if(Serial0_buffer=="mat"){
      get_number = fg_matchFinger();
      Serial0.print("match id:");
      Serial0.println(get_number);
    }else if(Serial0_buffer=="num"){
      get_number = fg_getNumber();
      Serial0.print("number:");
      Serial0.println(get_number);
    }else if(Serial0_buffer=="det"){
      fg_deteleFinger();
    }else if(Serial0_buffer=="fre"){
      get_number = fg_getFreeID();
      Serial0.print("free ID:");
      Serial0.println(get_number);
    }
  }
}
//指纹串口1中断
void Serial1_interruption(void){
  Serial1_buffer.clear();
  if (xSemaphoreTake(Serial1_buffer_Mutex, portMAX_DELAY)) {
    uint32_t now = millis();
    while ((millis() - now) < communicationTimeout_ms) {
      if (Serial1.available()) {
        Serial1_buffer += (char)Serial1.read();
        now = millis();
      }
    }
    xSemaphoreGive(Serial1_buffer_Mutex);
    if(RX_flag==0){
      RX_flag=1;
    }
  }
}

//指纹模块串口初始化
void fingerprinter_init(void){
  //串口初始化
  Serial0.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  //串口1中断初始化
  Serial1_buffer_Mutex = xSemaphoreCreateMutex();
  if (Serial1_buffer_Mutex == NULL) {
    log_e("Error creating Mutex. Sketch will fail.");
    while (true) {
      Serial0.println("Mutex error (NULL). Program halted.");
      delay(2000);
    }
  }

  //串口0中断初始化
  Serial0_buffer_Mutex = xSemaphoreCreateMutex();
  if (Serial0_buffer_Mutex == NULL) {
    log_e("Error creating Mutex. Sketch will fail.");
    while (true) {
      Serial0.println("Mutex error (NULL). Program halted.");
      delay(2000);
    }
  }

  //连接中断函数
  Serial0.onReceive(Serial0_interruption);
  Serial1.onReceive(Serial1_interruption);

  Serial0.println("The serial port initialization succeeds.");
}

//发射命令
uint8_t fg_SendCMD(uint8_t position,uint16_t out_time){
  unsigned long time,start_time;
  RX_flag=0;
  Serial1.write(FingerPrint_CMD[position],8);
  start_time = millis();
  while(RX_flag==0){
    if(millis()-start_time>=out_time && out_time!=0){
      Serial0.println("connect fail!");
      return 0;
    }
    delay(20);
  }
  Serial0.println("get sucessed!");
  return 1;
}

void fg_getVersion(void){
  if(fg_SendCMD(0,0)){
    Serial0.println(Serial1_buffer);
  }
}
uint8_t fg_fingerReg(void){
  uint8_t i;
  fg_SendCMD(1,0);
  delay(2000);
  fg_SendCMD(1,0);
  delay(2000);
  if(fg_SendCMD(1,0)){
    if(Serial1_buffer[5]!=0){
      Serial0.println("reg fail");
      return 0;
    }else{
        if(Serial1_buffer[3]>0){
            Serial0.print("reg successed");
            return Serial1_buffer[3];
        }else{
            Serial0.println("reg fail");
            return 0;
        }
    }
  }
  return 0;
}
void fg_deteleFinger(void){
  if(fg_SendCMD(2,800)){
    Serial0.println("detele successed");
  }
}
uint8_t fg_getFreeID(void){
  if(fg_SendCMD(3,500)){
    return Serial1_buffer[3];
  }
  return 0;
}
uint8_t fg_matchFinger(void){
  if(fg_SendCMD(4,0)){
    if(Serial1_buffer[4]==0x01){
      return Serial1_buffer[3];
    }
  }
  return 0;
}
uint8_t fg_getNumber(void){
  if(fg_SendCMD(5,200)){
    return Serial1_buffer[3];
  }
  return 0;
}
void fg_getImage(void){
  if(fg_SendCMD(6,200)){
  Serial0.printf("%X\r\n",Serial1_buffer);
  }
}

#endif