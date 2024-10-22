#include "fingerprinter.h"

void setup() {
  fingerprinter_init();
}

void loop() {
  fg_getversion();
  delay(3000);
}
