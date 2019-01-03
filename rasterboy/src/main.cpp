#include <mbed.h>
#include <TextLCD.h>

#include "StringBuilder.h"

DigitalOut statusLed(PC_13);
Serial debugLog(PA_9, PA_10, "debug_log", 115200);

BusIn modeSel(PA_6, PA_7, PB_0);
BusIn channelSel(PB_12, PB_13, PB_14, PB_15);
BusOut muxCtl(PA_2, PA_3, PA_4, PA_5);
PwmOut powerLedCtl(PA_8);
Serial midiInOut(PB_10, PB_11, 30000);
//I2C i2c(PB_8, PB_9);

//TextLCD lcd(&i2c, 126, TextLCD::LCD16x2);

int main() {
  statusLed = 0;

  modeSel.mode(PullUp);
  channelSel.mode(PullNone);

  powerLedCtl.period_us(2500);
  powerLedCtl.pulsewidth_us(2500);

  statusLed = 1;

  debugLog.puts("Rasterboy inited\r\n");

  int lastMode = -1;
  int lastChannel = -1;

  while(true) {
    int mode = modeSel.read();
    int channel = channelSel.read();

    if((mode != lastMode) || (channel != lastChannel)) {
      lastMode = mode;
      lastChannel = channel;

      //lcd.cls();
      StringBuilder<16> sb;
      sb.append(" M="); sb.appendUHex(mode); sb.append(" C="); sb.appendUHex(channel);
      debugLog.puts(sb.str()); debugLog.puts("\r\n");
      //lcd.puts(sb.str()); lcd.putc('\n');
    }
  }
  debugLog.puts("Done!\r\n");

  return 0;
}
