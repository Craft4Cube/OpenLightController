# OpenLightController
OpenLightController is an I2C controlled, RGB-Strip controller. It does RGB fades and gradients without extra load on the I2C master. 

# Protocol
OpenLightController uses the I2C standard for communications. By default it listens on address 0x10.

The commands are simply written to the register 0x00 as I2C blocks.
