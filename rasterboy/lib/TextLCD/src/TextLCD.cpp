/* mbed TextLCD Library, for a 4-bit LCD based on HD44780
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TextLCD.h"

#include "mbed.h"

#include <new>

TextLCD::TextLCD(PinName rs, PinName e, PinName d4, PinName d5,
                 PinName d6, PinName d7, LCDType type) :
        _type((LCDType)((type & ~IFMask) | IFGPIO4BIT)) {
    new((GPIOData *)_raw) GPIOData(rs, e, d4, d5, d6, d7);
    init();
}

TextLCD::TextLCD(I2C * i2c, uint8_t i2cAddress, LCDType type) :
        _type((LCDType)((type & ~IFMask) | IFPCF8574)) {
    new((GPIOData *)_raw) PCF8574Data(i2c, i2cAddress);
    ((PCF8574Data *)_raw)->setPins(0xFF, 0xF0 | 0x08 | 0x01);
    init();
}

TextLCD::~TextLCD()
{
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->~GPIOData();
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->~PCF8574Data();
            break;
    }
}

void TextLCD::init()
{
    setE();
    clearRs();           // command mode

    wait_ms(15);        // Wait 15ms to ensure powered up

    // send "Display Settings" 3 times (Only top nibble of 0x30 as we've got 4-bit bus)
    for (int i=0; i<3; i++) {
        writeByte(0x3);
        wait_us(1640);  // this command takes 1.64ms, so wait for it
    }
    writeByte(0x2);     // 4-bit mode
    wait_us(40);    // most instructions take 40us

    writeCommand(0x28); // Function set 001 BW N F - -
    writeCommand(0x0C);
    writeCommand(0x6);  // Cursor Direction and Display Shift : 0000 01 CD S (CD 0-left, 1-right S(hift) 0-no, 1-yes
    cls();
}

void TextLCD::character(int column, int row, int c) {
    int a = address(column, row);
    writeCommand(a);
    writeData(c);
}

void TextLCD::cls() {
    writeCommand(0x01); // cls, and set cursor to 0
    wait(0.00164f);     // This command takes 1.64 ms
    locate(0, 0);
}

void TextLCD::locate(int column, int row) {
    _column = column;
    _row = row;
}

int TextLCD::_putc(int value) {
    if (value == '\n') {
        _column = 0;
        _row++;
        if (_row >= rows()) {
            _row = 0;
        }
    } else {
        character(_column, _row, value);
        _column++;
        if (_column >= columns()) {
            _column = 0;
            _row++;
            if (_row >= rows()) {
                _row = 0;
            }
        }
    }
    return value;
}

int TextLCD::_getc() {
    return -1;
}

void TextLCD::writeByte(int value) {
    setD(value >> 4);
    wait_us(40); // most instructions take 40us
    clearE();
    wait_us(40);
    setE();
    setD(value >> 0);
    wait_us(40);
    clearE();
    wait_us(40);  // most instructions take 40us
    setE();
}

void TextLCD::writeCommand(int command) {
    clearRs();
    writeByte(command);
}

void TextLCD::writeData(int data) {
    setRs();
    writeByte(data);
}

int TextLCD::address(int column, int row) {
    switch (_type & LCDMask) {
        case LCD20x4:
            switch (row) {
                case 0:
                    return 0x80 + column;
                case 1:
                    return 0xc0 + column;
                case 2:
                    return 0x94 + column;
                case 3:
                    return 0xd4 + column;
            }
        case LCD16x2B:
            return 0x80 + (row * 40) + column;
        case LCD16x2:
        case LCD20x2:
        default:
            return 0x80 + (row * 0x40) + column;
    }
}

int TextLCD::columns() {
    switch (_type & LCDMask) {
        case LCD20x4:
        case LCD20x2:
            return 20;
        case LCD16x2:
        case LCD16x2B:
        default:
            return 16;
    }
}

int TextLCD::rows() {
    switch (_type & LCDMask) {
        case LCD20x4:
            return 4;
        case LCD16x2:
        case LCD16x2B:
        case LCD20x2:
        default:
            return 2;
    }
}

void TextLCD::setRs() {
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->_rs = 1;
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->setPins(0x01, 0x01);
            break;
    }
}

void TextLCD::clearRs() {
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->_rs = 0;
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->setPins(0x01, 0x00);
            break;
    }
}

void TextLCD::setE() {
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->_e = 1;
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->setPins(0x04, 0x04);
            break;
    }
}

void TextLCD::clearE() {
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->_e = 0;
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->setPins(0x04, 0x00);
            break;
    }
}

void TextLCD::setD(int val) {
    switch(_type & IFMask) {
        case IFGPIO4BIT:
            ((GPIOData *)_raw)->_d = val & 0x0F;
            break;
        case IFPCF8574:
            ((PCF8574Data *)_raw)->setPins(0xF0, (val << 4) & 0xF0);
            break;
    }
}
