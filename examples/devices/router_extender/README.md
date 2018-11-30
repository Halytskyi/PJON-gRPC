**router_extender** - communication between RPi and devices with different strategies

RPi <—--ThroughSerialAsync---> Arduino (router) <—--SoftwareBitBang--—> Arduino (device1)
                                                |
                                                <---ThroughSerialAsync (HC-12)---> Arduino (device2)
                                                |
                                                <--- Other PJON strategy---> Arduino (deviceN)

**router** - firmware for Arduino router which allow RPi communicate with devices connected to one PJON bus, see device example: [device1](device1/device1.ino)

**router_2_bus** - firmware for Arduino router which allow RPi communicate with devices connected to two different PJON bus, see device examples: [device1](device1/device1.ino) and [device2](device2/device2.ino)

**device1** - firmware for Arduino device which connected to Arduino router via '[SoftwareBitBang](https://github.com/gioblu/PJON/tree/master/src/strategies/SoftwareBitBang)' strategy

**device2** - firmware for Arduino device which connected to Arduino router via '[ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync)' ([HC-12 module](http://statics3.seeedstudio.com/assets/file/bazaar/product/HC-12_english_datasheets.pdf))
