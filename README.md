# PJON-gRPC

PJON-gRPC is a server-client interface for linux-based machines (applications can be written on [different languages](https://grpc.io/docs/)) and devices Arduino, ATtiny, ATmega, ESP8266, etc.

## Components

**PJON™** (Padded Jittering Operative Network) is an Arduino compatible, multi-master, multi-media communications bus system created and mantained by Giovanni Blu Mitolo gioscarab@gmail.com https://github.com/gioblu/PJON

**gRPC** is a modern open source high performance RPC framework that can run in any environment https://github.com/grpc/grpc

## Server features

- Receive requests from application located on RPi/PC (via gRPC, for example [pjon_grpc_client.py](examples/clients/python/pjon_grpc_client.py)) and send back responses from devices (like Arduino) connected with RPi through PJON [ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync) strategy.
- Receive and forward messages to application located on RPi/PC from devices through PJON [ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync) strategy (via gRPC, for example [pjon_grpc_clientserver.py](examples/clients/python/pjon_grpc_clientserver.py))
- Communication between RPi/PC and devices (like Arduino) through any [PJON strategies](https://github.com/gioblu/PJON/blob/master/documentation/configuration.md) via [router](examples/devices/router_extender)
- Can be configured for working with 2 modules at the same time (see [pjon-grpc.cfg](server/conf/pjon-grpc.cfg)). Logic isolated in separate threads for maximum performance. Very useful if need to distribute the load, for example, `1st module` can be configured for accept and response on requests, `2nd module` can be configured for accept only incoming messages.

## Tested with

**PJON version:** [12.0](https://github.com/gioblu/PJON/tree/12.0)<br>
**Device:** [Raspberry Pi 3](https://www.raspberrypi.org/products/raspberry-pi-3-model-b/)<br>
**OS:** [Raspbian buster](https://www.raspberrypi.org/downloads/raspbian/)

## Video example (for old version, will be changed)

https://www.youtube.com/watch?v=J8FVPZW4y4I
