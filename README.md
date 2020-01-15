# PJON-gRPC

PJON-gRPC is a client-server application for linux-based machines for communication with remote devices like Arduino, ATtiny, ATmega, ESP8266, etc.

## Components

**PJON™** (Padded Jittering Operative Network) is an Arduino compatible, multi-master, multi-media communications bus system created and mantained by Giovanni Blu Mitolo gioscarab@gmail.com https://github.com/gioblu/PJON

**gRPC** is a modern open source high performance RPC framework that can run in any environment https://github.com/grpc/grpc

## Server features

- Receive requests from application located on RPi/PC (via gRPC, for example [pjon_grpc_client.py](examples/clients/python/pjon_grpc_client.py)) and send back responses from devices (like Arduino) connected with RPi through PJON [ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync) strategy.
- Receive and forward messages to application located on RPi/PC from devices through PJON [ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync) strategy (via gRPC, for example [pjon_grpc_clientserver.py](examples/clients/python/pjon_grpc_clientserver.py))
- Communication between RPi/PC and devices (like Arduino) through any [PJON strategies](https://github.com/gioblu/PJON/blob/master/documentation/configuration.md) via [router](examples/devices/router_extender)

For maximum stability and performance recommended to use separated modules for `transmit-receive` and `receive only` operations with separated physical busses. With this configuration wasn't detected any issues while sending/receiving messages each 0.2 seconds. See examples [two routers](examples/devices/two_routers)

## Tested with

**PJON version:** [12.0](https://github.com/gioblu/PJON/tree/12.0)<br>
**Device:** [Raspberry Pi 3](https://www.raspberrypi.org/products/raspberry-pi-3-model-b/)<br>
**OS:** [Raspbian buster](https://www.raspberrypi.org/downloads/raspbian/)

## Video for example described under [two routers](examples/devices/two_routers)

<video src="video/PJON-gRPC.mp4" width="640" height="360" autoplay loop preload controls></video>

[https://youtu.be/R4MZhWncfPs](https://youtu.be/R4MZhWncfPs)
