This sketch is example how can control PINs via PJON-gRPC tool

**Examples** (with use python client on RPi side):

Read Hardware Digital PIN (_digitalRead(13)_):
```
$ ./pjon_grpc_client.py 44 H-13
Client received: 0
```
Read Hardware Analog PIN (_analogRead(14)_):
```
$ ./pjon_grpc_client.py 44 H-14
Client received: 149
```
Write Hardware Digital PIN (_digitalWrite(13, 1)_):
```
$ ./pjon_grpc_client.py 44 H-13-1
Client received: 1
```
Read Virtual PIN:
```
$ ./pjon_grpc_client.py 44 V-0
Client received: 0
```
Write Virtual PIN:
```
./pjon_grpc_client.py 44 V-0-1
Client received: 1
```

`Makefile` - for compiling code and flashing it to Arduino directly from RPi (see https://github.com/sudar/Arduino-Makefile)
