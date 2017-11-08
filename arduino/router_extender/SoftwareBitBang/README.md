This sketch is example how can control PINs via SoftwareBitBang strategy from RPi through Arduino-router

**Examples** (with use python client on RPi side):

Read Hardware Digital PIN (_digitalRead(13)_):
```
$ ./pjon_grpc_client.py 11 H:13
Client received: 0
```
Read Hardware Analog PIN (_analogRead(14)_):
```
$ ./pjon_grpc_client.py 11 H:14
Client received: 149
```
Write Hardware Digital PIN (_digitalWrite(13, 1)_):
```
$ ./pjon_grpc_client.py 11 H:13:1
Client received: 1
```

`Makefile` - for compiling code and flashing it to Arduino directly from RPi (see https://github.com/sudar/Arduino-Makefile)
