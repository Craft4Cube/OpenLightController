/* 
 * This file is part of the OpenLightController distribution (https://github.com/Craft4Cube/OpenLightController).
 * Copyright (c) 2020 Lukas Schmid.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Wire.h>
#include <FastLED.h>

//**** User Configuration ****
#define LED_DATA    10
#define LED_TYPE    WS2812
#define LED_COUNT   32
#define SLAVE_ADDR  0x10
//** End User Configuration **

CRGB ledOutputBuffer[LED_COUNT];      //Current RGB values to be written out.
CRGB ledTargetBuffer[LED_COUNT];      //Target RGB values to be faded to.
CRGB ledOriginBuffer[LED_COUNT];      //Previous RGB valuse to be faded from.

byte ledStepsLeft[LED_COUNT];         //Steps left until fade is complete
byte ledTotalSteps[LED_COUNT];        //Amount of steps to pass for complete fade
byte ledStepTime[LED_COUNT];          //Time left until next step
byte ledStepDelay[LED_COUNT];         //Time between steps in ms

volatile byte registerIndex;          //Current selected I2C register
volatile byte registers[256];         //I2C registers

enum Commands {
  NoOperation =         0x00,
  SetLedColorRGB =      0x01,
  SetLedColorRangeRGB = 0x02,
  SetLedColorHSV =      0x11,
  SetLedColorRangeHSV = 0x12,
  ClearOutputBuffer   = 0xFF
};

void setup() {
  //Initialize I2C bus in slave mode, and register event handlers
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  //Initialize leds
  FastLED.addLeds<WS2812, LED_DATA, RGB>(ledOutputBuffer, LED_COUNT);
}

void loop() {
  //Handle I2C commands
  switch (registers[0]) {
    default: case NoOperation:
      break;

    case SetLedColorRGB:
      setLedTargetColor(registers[1], CRGB(registers[2], registers[3], registers[4]), registers[5], registers[6]);
      registers[0] = NoOperation;
      break;  

    case SetLedColorRangeRGB:
      setLedTargetRangeColor(registers[1], registers[2], CRGB(registers[3], registers[4], registers[5]), CRGB(registers[6], registers[7], registers[8]), registers[9], registers[10]);
      registers[0] = NoOperation;
      break;  

    case SetLedColorHSV:
      setLedTargetColor(registers[1], CHSV(registers[2], registers[3], registers[4]), registers[5], registers[6]);
      registers[0] = NoOperation;
      break;  

    case SetLedColorRangeHSV:
      setLedTargetRangeColor(registers[1], registers[2], CHSV(registers[3], registers[4], registers[5]), CHSV(registers[6], registers[7], registers[8]), registers[9], registers[10]);
      registers[0] = NoOperation;
      break;  

    case ClearOutputBuffer:
      setLedTargetRangeColor(0, LED_COUNT, CRGB::Black, CRGB::Black, 0, 1);
      registers[0] = NoOperation;
      break;  
  }

  //Actually fade every single led
  for (byte led = 0; led < LED_COUNT; led++) {
    
    //Are there any steps left to fade
    if (ledStepsLeft[led] > 0) {
      
      //Do we still have to wait
      if (ledStepTime[led] == 0) {
        //We do not have to wait anymore, therfore 
        //decrement the current step and reset the time value
        ledStepsLeft[led]--;
        ledStepTime[led] = ledStepDelay[led];
      } else {
        //We still have to wait, therfore just decrement the time value
        ledStepTime[led]--;
      }

      //Calculate fade percentage
      float blendPercentage = ledStepsLeft[led];
      blendPercentage /= ledTotalSteps[led];

      //Set the new output color
      ledOutputBuffer[led] = blend(ledOriginBuffer[led], ledTargetBuffer[led], 255 - (blendPercentage * 255));
    }
  }

  //Write the output buffer to the leds
  FastLED.show();

  //Wait 1 ms
  delay(1);
}

//Called when I2C master tries to read a value
void requestEvent() {
  Wire.write(registers[registerIndex]);
  registerIndex++;
}

//Called when I2C master tries to write a value
void receiveEvent(int len) {
  //Read first byte as index
  registerIndex = Wire.read();

  //If more bytes are available, write the to memory at specified index
  while (Wire.available()) {
    byte value = Wire.read();
    registers[registerIndex] = value;
    registerIndex++;
  }
}

void setLedTargetColor(byte led, CRGB targetColor, byte stepDelay, byte stepCount) {
  //Set led origin color to current color
  ledOriginBuffer[led] = ledOutputBuffer[led];

  //Set target color to specified color
  ledTargetBuffer[led] = targetColor;

  //Set total steps to step count with minimum of 1
  ledTotalSteps[led] = max(stepCount, 1);

  //Set current steps to step count with minimum of 1
  ledStepsLeft[led] = max(stepCount, 1);

  //Set step delay to sepcified delay
  ledStepDelay[led] = stepDelay;

  //Set step time left to specified delay
  ledStepTime[led] = stepDelay;
}

void setLedTargetRangeColor(byte ledMin, byte ledMax, CRGB targetMinColor, CRGB targetMaxColor, byte stepDelay, byte stepCount) {
  //Cap led values to total led count
  ledMax = min(ledMax, LED_COUNT);
  ledMin = min(ledMin, LED_COUNT);

  //For each led in given range
  for (byte led = ledMin; led < ledMax; led++) {
    
    //Get led count relative to current range
    float ledCount = ledMax - ledMin;

    //Get led index relative to current range
    float ledIndex = led - ledMin;

    //Get led position in current range
    float percentage = ledIndex / ledCount;

    //Calculate color for current led
    CRGB targetColor = nblend(targetMinColor, targetMaxColor, percentage);

    //Set led target color
    setLedTargetColor(led, targetColor, stepDelay, stepCount);
  }
}
