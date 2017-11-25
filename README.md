# PJON-gRPC
PJON-gRPC is a server-client interface for linux-based machines (scripts can be written on [different languages](https://grpc.io/docs/)) and devices Arduino, ATtiny, ATmega, ESP8266, etc.

PJON™ (Padded Jittering Operative Network) is an Arduino compatible, multi-master, multi-media communications bus system created and mantained by Giovanni Blu Mitolo gioscarab@gmail.com https://github.com/gioblu/PJON

gRPC is a modern open source high performance RPC framework that can run in any environment https://github.com/grpc/grpc

### Features
- Receive requests from client application (via gRPC, for example [pjon_grpc_client.py](client/python/pjon_grpc_client.py)) and send back responses from devices connected with RPi through PJON SoftwareBitBang strategy.
- Receive and forward to client messages from devices through PJON SoftwareBitBang strategy (via gRPC, for example [pjon_grpc_clientserver.py](client/python/pjon_grpc_clientserver.py))

#### Video example
https://www.youtube.com/watch?v=J8FVPZW4y4I
