#include <CAN.h>
#include <SPI.h>
#include <stdint.h>

int speedSendPeriod = 72;
int keepAliveSendPeriod = 2000;

unsigned long speedSendElapsed, kaSendElapsed = 0;

int targetSpeed, kaCounter, keepAliveCounter = 0;

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned long __u32;

struct can_frame {
    __u32   can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __u8    can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
    __u8    data[8] __attribute__((aligned(8)));
};

struct can_frame trwKeepAlive, trwSpeed;

byte kaVar[4] = {0x00, 0x40, 0x80, 0xC0};

void setup() {
  trwKeepAlive.can_id  = 0x1ae0092c;
  trwKeepAlive.can_dlc = 8;
  trwKeepAlive.data[0] = 0x00;
  trwKeepAlive.data[1] = 0x00;
  trwKeepAlive.data[2] = 0x22;
  trwKeepAlive.data[3] = 0xe0;
  trwKeepAlive.data[4] = 0x41;
  trwKeepAlive.data[5] = 0x90;
  trwKeepAlive.data[6] = 0x00;
  trwKeepAlive.data[7] = 0x00;  
  
  trwSpeed.can_id  = 0x02104136;
  trwSpeed.can_dlc = 8;
  trwSpeed.data[0] = 0xbb;
  trwSpeed.data[1] = 0x00;
  trwSpeed.data[2] = 0x3f;
  trwSpeed.data[3] = 0xff;
  trwSpeed.data[4] = 0x06;
  trwSpeed.data[5] = 0xe0;
  trwSpeed.data[6] = 0x00;
  trwSpeed.data[7] = 0x00;  
  
  Serial.begin(115200);

  delay(10);
  
  Serial.println("Start serial");
  
  CAN.setPins(26, 25);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  delay(100);
  
}


void loop() {
  //read CAN message
  int packetSize = CAN.parsePacket();
  if (packetSize) { //if there's a packet
    if(CAN.packetId() == 0x1B200002){ //message id from pump
      Serial.print("message from pump!");
    }
  }
  
  //send can keep alive packet every 2s
  if(millis() >= kaSendElapsed + keepAliveSendPeriod){
      kaSendElapsed += keepAliveSendPeriod;
      
      keepAliveCounter = (keepAliveCounter + 1) & 3;
      trwKeepAlive.data[0] = kaVar[keepAliveCounter];
      CAN.beginExtendedPacket(trwKeepAlive.can_id);
      CAN.write(trwKeepAlive.data, trwKeepAlive.can_dlc);
      CAN.endPacket(); 
    
      Serial.println("trwKeepAlive");
  }
  
  //send speed packet every 72ms
  if(millis() >= speedSendElapsed + speedSendPeriod){
      speedSendElapsed += speedSendPeriod;

      //set targetSpeed from 1-6000, lower number = faster pump speed
      targetSpeed = 1;

      trwSpeed.data[6] = (targetSpeed >> 8) & 0xFF;
      trwSpeed.data[7] = targetSpeed & 0xFF;
    
      CAN.beginExtendedPacket(trwSpeed.can_id);
      CAN.write(trwSpeed.data, trwSpeed.can_dlc);
      CAN.endPacket(); 
    
      Serial.print("trw send speed");
      Serial.print(trwSpeed.data[0],HEX);
      Serial.print(trwSpeed.data[1],HEX); 
      Serial.print(trwSpeed.data[2],HEX);
      Serial.print(trwSpeed.data[3],HEX);
      Serial.print(trwSpeed.data[4],HEX);
      Serial.print(trwSpeed.data[5],HEX);
      Serial.print(trwSpeed.data[6],HEX);
      Serial.print(trwSpeed.data[7],HEX);
      Serial.println("");

    }
  
}
