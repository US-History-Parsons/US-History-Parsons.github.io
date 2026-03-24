#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4

#define CLK_PIN   3
#define DATA_PIN  2
#define CS_PIN    4

byte smiley[8][8] = {
  {0x3c,0x42,0xa5,0x81,0xa5,0x99,0x42,0x3c},
  {0x3c,0x42,0x89,0xa1,0xa5,0x91,0x42,0x3c},
  {0x3c,0x42,0x95,0xa1,0xa1,0x95,0x42,0x3c},
  {0x3c,0x42,0x91,0xa5,0xa1,0x89,0x42,0x3c},
  {0x3c,0x42,0x99,0xa5,0x81,0xa5,0x42,0x3c},
  {0x3c,0x42,0x99,0x85,0xa1,0x89,0x42,0x3c},
  {0x3c,0x42,0xa9,0x85,0x85,0xa9,0x42,0x3c},
  {0x3c,0x42,0x89,0xa1,0x85,0x99,0x42,0x3c}
};

MD_MAX72XX display = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

byte silo[8 * MAX_DEVICES];

void setup() {
  Serial.begin(115200);
  display.begin();
  display.control(MD_MAX72XX::INTENSITY, 8);
  display.clear();

  for(int i = 0; i < 8 * MAX_DEVICES; i++) {
    silo[i] = 0;
  }
}

void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}



void simulate() {
  silo[0] |= 1 << random(8);

  byte temp[8 * MAX_DEVICES];
  copy(silo, temp, 8 * MAX_DEVICES);

  for(int i = 0; i < 8 * MAX_DEVICES - 1; i++) {
    for(int bit = 0; bit < 8; bit++) {
      bool thisr = bitRead(silo[i], bit);

      if(thisr) {
        if(!bitRead(silo[i+1], bit)) {
          temp[i+1] |= bit(bit);
          temp[i] &= ~bit(bit);
        } else if(bit < 7 && !bitRead(silo[i+1], bit+1)) {
          temp[i+1] |= bit(bit+1);
          temp[i] &= ~bit(bit);
        } else if(bit > 0 && !bitRead(silo[i+1], bit-1)) {
          temp[i+1] |= bit(bit-1);
          temp[i] &= ~bit(bit);
        }
      }
    }
  }

  copy(temp, silo, 8 * MAX_DEVICES);
}

byte lastFrame[32];

void frame() {
  for(int b = MAX_DEVICES - 1; b >= 0; b--) {
    for(int i = 0; i < 8; i++) {
      if(silo[b * 8 + 7 - i] != lastFrame[b * 8 + 7 - i]) {
        lastFrame[b * 8 + 7 - i] = silo[b * 8 + 7 - i];
        display.setRow(b, i, silo[b * 8 + 7 - i]);
      }
    }
  }
}

#define TICK_TIME 10
#define FRAME_TICK 5
int tick = 0;
void loop() {
  long start = millis();
  simulate();
  tick++;
  if(tick >= FRAME_TICK) {
    frame();
    tick = 0;
  }
  long dt = millis() - start;
  if(dt < TICK_TIME){
    delay(TICK_TIME - dt);
  }
}