**router_extender** - communication between RPi and devices with different strategies

RPi <—--ThroughSerial---> Arduino (router) <—--SoftwareBitBang--—> Arduino (device1)

**router** - firmware for Arduino router

**device1** - firmware for Arduino device which connected to Arduino router via '[SoftwareBitBang](https://github.com/gioblu/PJON/tree/master/src/strategies/SoftwareBitBang)' strategy
