#include "HardwareSerial.h"
#ifndef _FINGERPRINTER_H_
#define _FINGERPRINTER_H_

#include "Arduino.h"

//#define FINGERPRINTER_UART Serial0  //USB串口
#define FINGERPRINTER_UART Serial1    //17TX,18RX

const uint32_t communicationTimeout_ms = 10;
String uart_buffer;
FingerPrinter finger;

enum fingerprinter_cmd_enum{
  fg_stop,
  fg_getver=1,
  fg_reg=2,
  fg_delete=3,
  fg_getid=4,
  fg_match=5,
  fg_getcount=6,
  fg_getimage=,
  fg_setbaud=8,
};

typedef struct{
  fingerprinter_cmd_enum current_behavior;
  void (*RX_interruption)(String RX_buf);
  int RX_counter;
  SemaphoreHandlet uart_buffer_Mutex = NULL;
}FingerPrinter;
FingerPrinter finger;

uint8_t FingerPrint_CMD[7][8] = {
  {0xF5,0x26,0x00,0x00,0x00,0x00,0x26,0xF5},//version
  {0xF5,0x01,0x00,0x00,0x01,0x00,0x00,0xF5},//reg
  {0xF5,0x05,0x00,0x00,0x00,0x00,0x05,0xF5},//delete
  {0xF5,0x0D,0x00,0x00,0x00,0x00,0x0D,0xF5},//ID
  {0xF5,0x0C,0x00,0x00,0x00,0x00,0x0C,0xF5},//match
  {0xF5,0x09,0x00,0x00,0x00,0x00,0x09,0xF5},//count
  {0xF5,0x24,0x00,0x00,0x00,0x00,0x24,0xF5},//image
};

void uart_processing_data(String RX_buf){
  if(finger.current_behavior != fg_stop){
    if(finger.current_behavior == fg_getver){
      Serial.println(RX_buf);
      finger.current_behavior = 0;
      
    }
  }
}

void uart_interruption(void){
  if (xSemaphoreTake(finger.uart_buffer_Mutex, portMAX_DELAY)) {
    uint32_t now = millis();
    while ((millis() - now) < communicationTimeout_ms) {
      if (FINGERPRINTER_UART.available()) {
        uart_buffer += (char)FINGERPRINTER_UART.read();
        now = millis();
      }
    }
    xSemaphoreGive(finger.uart_buffer_Mutex);
    finger.RX_interruption(uart_buffer);
  }
}

void fingerprinter_init(void){
  FINGERPRINTER_UART.begin(115200);
  Serial.begin(115200);
  finger.uart_buffer_Mutex = xSemaphoreCreateMutex();
  if (finger.uart_buffer_Mutex == NULL) {
    log_e("Error creating Mutex. Sketch will fail.");
    while (true) {
      Serial.println("Mutex error (NULL). Program halted.");
      delay(2000);
    }
  }
  finger.RX_interruption=u art_processing_data;
  FINGERPRINTER_UART.onReceive(uart_interruption);
  FINGERPRINTER_UART.println("Send data to UART0 in order to activate the RX callback");
}

void fg_getversion(void){
  FINGERPRINTER_UART.write(FingerPrint_CMD[0],8);
  finger.current_behavior = fg_getver;
  Serial.println("get version...");
}

#endif