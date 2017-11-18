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
Make test, number in answers will be increased on 1:
```
$ for i in $(seq 1 2); do ./pjon_grpc_client.py 11 T:1; done
Client received: 1
Client received: 2
```
Enable test, Arduino will send consistently numbers to RPi (device 1)
```
$ ./pjon_grpc_client.py 11 T:2:1
Client received: 1
```

`Makefile` - for compiling code and flashing it to Arduino directly from RPi (see https://github.com/sudar/Arduino-Makefile)
