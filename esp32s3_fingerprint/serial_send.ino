/**
此项目是适用于TM1026指纹模块的简易程序,只实习了部分基础功能
库文件在fingerprint.h
使用的开发板为esp32s3 pico,串口对应端口为17TX,18RX,可以根据需要更改
调试端口波特率为115200,指纹模块波特率为9600
如果指纹模块波特率不是9600,需要先更改指纹模块波特率
初始化后通过调用函数即可
此例程是使用串口助手向Serial0发送数据调用函数,具体看Serial0_interruption
**/

#include "fingerprinter.h"

void setup() {
  fingerprinter_init();
}

void loop() {
  delay(1000);
}
