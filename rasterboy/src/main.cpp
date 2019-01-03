#include <mbed.h>
#include <TextLCD.h>

#include "StringBuilder.h"

DigitalOut statusLed(PC_13, 0);

Serial debugUart(PA_9, PA_10, "debug_log", 115200);

DigitalIn snapshotButton(PB_6);
BusIn modeSwitch(PA_4, PA_5, PA_6);
BusIn channelSwitch(PB_12, PB_13, PB_14, PB_15);
BusOut muxSelect(PA_0, PA_1, PA_2, PA_3);
PwmOut powerLedDrive(PA_8);
Serial midiUart(PB_10, PB_11, 30000);
DigitalOut adcNcs(PA_15, 1);
SPI adcSerial(NC, PB_4, PB_3);
I2C i2c(PB_9, PB_8);

// For this to work we'll need DMA, which mbed doesn't have...
// It can be done, but via the HAL
//SPI ws2811(PA_7, NC, NC);
//  ws2811.frequency(5000000UL);
//  ws2811.set_dma_usage(DMA_USAGE_ALWAYS);

TextLCD lcd(&i2c, 126, TextLCD::LCD16x2);

int main() {
  modeSwitch.mode(PullUp);
  channelSwitch.mode(PullNone);

  powerLedDrive.period_us(2500);
  powerLedDrive.pulsewidth_us(1250);

  adcSerial.frequency(1000000UL);

  statusLed = 1;

  debugUart.puts("Rasterboy inited\r\n");

  bool lastSnapshot = false;

  while(true) {
    int mode = modeSwitch.read();
    int channel = ~channelSwitch.read() & 0x0F;
    int snapshot = !snapshotButton.read();
    size_t const NUM_KNOBS = 13;
    uint8_t knobVals[NUM_KNOBS];

    for(size_t i = 0; i < NUM_KNOBS + 1; ++i) {
      muxSelect.write(i);
      wait_us(10);
      adcNcs.write(0);
      wait_us(2);
      uint8_t val = (uint8_t)adcSerial.write(0);
      adcNcs.write(1);
      if(i > 0) {
        knobVals[i - 1] = val;
      }
    }

    while(midiUart.readable()) {
      uint8_t val = midiUart.getc();
      midiUart.putc(val);
      StringBuilder<10> sb;
      sb.append("m"); sb.appendUHex(val);
      debugUart.puts(sb.str());
    }

    if(snapshot && !lastSnapshot) {
      {
        lcd.cls();
        StringBuilder<16> sb;
        sb.append(" M="); sb.appendUHex(mode);
        sb.append(" C="); sb.appendUHex(channel);
        sb.append(" S="); sb.appendU(snapshot);
        lcd.puts(sb.str()); lcd.putc('\n');
        sb.reset();
        sb.append(" K0="); sb.appendUHex(knobVals[0]);
        sb.append(" K1="); sb.appendUHex(knobVals[1]);
        lcd.puts(sb.str()); lcd.putc('\n');
      }
      {
        StringBuilder<79> sb;
        sb.append(" M="); sb.appendUHex(mode);
        sb.append(" C="); sb.appendUHex(channel);
        sb.append(" S="); sb.appendU(snapshot);
        for(size_t i = 0; i < NUM_KNOBS; ++i) {
          sb.append(" K"); sb.appendU(i); sb.append("="); sb.appendUHex(knobVals[i]);
        }
        debugUart.puts(sb.str()); debugUart.puts("\r\n");
      }
    }
    lastSnapshot = snapshot;
  }
  debugUart.puts("Done!\r\n");

  return 0;
}
