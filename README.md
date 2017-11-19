# PJON-gRPC
gRPC server-client for PJON bus - tool for communication between linux-based clients (scripts can be written on [different languages](https://grpc.io/docs/)) and devices, like: Arduino, ATtiny, ATmega, ESP8266, etc.

PJON™ (Padded Jittering Operative Network) is an Arduino compatible, multi-master, multi-media communications bus system created and mantained by Giovanni Blu Mitolo gioscarab@gmail.com https://github.com/gioblu/PJON

gRPC is a modern open source high performance RPC framework that can run in any environment https://github.com/grpc/grpc

### Features
- Receiving requests from client application (via gRPC, for example [pjon_grpc_client.py](client/python/pjon_grpc_client.py)) and making back response from devices which communicate with RPi via PJON
- Receiving messages from devices which sending them to RPi via PJON and forward these messages to client (via gRPC, for example [pjon_grpc_clientserver.py](client/python/pjon_grpc_clientserver.py))

#### Video example
https://www.youtube.com/watch?v=J8FVPZW4y4I
