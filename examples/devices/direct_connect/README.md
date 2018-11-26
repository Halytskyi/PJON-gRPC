This example show how possible control Arduino PINs or send messages to RPi/PC via PJON-gRPC tool. Arduino connected directly to RPi through serial (USB).

### Examples
Note: use python client '[pjon_grpc_client.py](examples/clients/python/pjon_grpc_client.py)' on RPi/PC side):

Read Hardware Digital PIN (_digitalRead(13)_):
```
$ ./pjon_grpc_client.py 1 11 H-13
Client received: H-13>0
```
Read Hardware Analog PIN (_analogRead(14)_):
```
$ ./pjon_grpc_client.py 1 11 H-14
Client received: H-14>275
```
Write Hardware Digital PIN (_digitalWrite(13, 1)_):
```
$ ./pjon_grpc_client.py 1 11 H-13-1
Client received: H-13-1>1
```
Read Virtual PIN:
```
$ ./pjon_grpc_client.py 1 11 V-0
Client received: V-0>0
```
Write Virtual PIN:
```
$ ./pjon_grpc_client.py 1 11 V-0-1
Client received: V-0-1>1
```
Read again the same PIN (after write):
```
$ ./pjon_grpc_client.py 1 11 V-0
Client received: V-0>1
```
Wrong command:
```
$ ./pjon_grpc_client.py 1 11 T
Client received: T>wrong command
```

#### Example of sending messages Arduino -> RPi/PC. Use '[pjon_grpc_clientserver.py](examples/clients/python/pjon_grpc_clientserver.py)' for tests
Enable sending messages every 1 second from Arduino to RPi/PC:
```
$ ./pjon_grpc_client.py 1 11 M-1
Client received: M-1>1
```
On RPi/PC side we should see:
```
$ ./pjon_grpc_clientserver.py
Client-Server received: node_id=11, data=<Incoming message every 1 sec
Client-Server received: node_id=11, data=<Incoming message every 1 sec
...
```

`Makefile` - for compiling code and flashing it to Arduino directly from RPi (see https://github.com/sudar/Arduino-Makefile)
