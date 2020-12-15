# OpenLightController
OpenLightController is an I2C controlled, RGB-Strip controller. It does RGB fades and gradients without extra load on the I2C master. 

## Note
OpenLightController reqiures the FastLED library. See http://fastled.io/

# Protocol
OpenLightController uses the I2C standard for communications. By default it listens on address 0x10.

The commands are simply written to the register 0x00 as I2C blocks.

## Instruction structure

```
<CMD> <ARG0> ... <ARGX>
```

## Command list

| Value | Name                | Arguments                                                                                                                           | Description                                                                                                                                   |
|-------|---------------------|-------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| 0x00  | No Operation        | None                                                                                                                                | Does nothing                                                                                                                                  |
| 0x01  | SetLedColorRGB      | [LED Index] [Red] [Green] [Blue] [Tick Delay] [Tick Count]                                                                          | Fades the led at [LED Index] to the given RGB-Color over [Tick Count] * [Tick Delay]                                                          |
| 0x02  | SetLedColorRangeRGB | [Start LED Index] [End LED Index] [Start Red] [Start Green] [Start Blue] [End Red] [End Green] [End Blue] [Tick Delay] [Tick Count] | Fades the leds from [Start LED Index] to [End LED Index] to a gradient calculated from the given RGB-Colors over [Tick Count] * [Tick Delay]  |
| 0x11  | SetLedColorHSV      | [LED Index] [Hue] [Saturation] [Value] [Tick Delay] [Tick Count]                                                                    | Fades the led at [LED Index] to the given HSV-Color over [Tick Count] * [Tick Delay]                                                          |
| 0x12  | SetLedColorRangeHSV | [Start LED Index] [End LED Index] [Start Red] [Start Green] [Start Blue] [End Red] [End Green] [End Blue] [Tick Delay] [Tick Count] | Fades the leds from  [Start LED Index] to [End LED Index] to a gradient calculated from the given HSV-Colors over [Tick Count] * [Tick Delay] |
| 0xFF  | ClearOutputBuffer   | None                                                                                                                                | Clears all buffers, and turns off all led's                                                                                                    |
