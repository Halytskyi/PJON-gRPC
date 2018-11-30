This example show how possible control Arduino PINs or send messages to RPi/PC via '[router](../router_1_bus)'

This device connected to '[router](../router_1_bus)' via '[SoftwareBitBang](https://github.com/gioblu/PJON/tree/master/src/strategies/SoftwareBitBang)' strategy

### Examples
**Note:** use python client '[pjon_grpc_client.py](../../../clients/python/pjon_grpc_client.py)' on RPi/PC side

#### Examples "request -> response"

Read Hardware Digital PIN (_digitalRead(13)_):
```
$ ./pjon_grpc_client.py 1 21 H-13
Client received: H-13>0
```
Read Hardware Analog PIN (_analogRead(14)_):
```
$ ./pjon_grpc_client.py 1 21 H-14
Client received: H-14>275
```
Write Hardware Digital PIN (_digitalWrite(13, 1)_):
```
$ ./pjon_grpc_client.py 1 21 H-13-1
Client received: H-13-1>1
```
Read Virtual PIN:
```
$ ./pjon_grpc_client.py 1 21 V-0
Client received: V-0>0
```
Write Virtual PIN:
```
$ ./pjon_grpc_client.py 1 21 V-0-1
Client received: V-0-1>1
```
Read again the same PIN (after write):
```
$ ./pjon_grpc_client.py 1 21 V-0
Client received: V-0>1
```
Make test, number in response will be increased on 1:
```
$ for i in $(seq 1 2); do ./pjon_grpc_client.py 1 21 N; done
Client received: N>1
Client received: N>2
```
Wrong command:
```
$ ./pjon_grpc_client.py 1 21 T
Client received: T>wrong command
```

#### Example of sending messages 'Arduino -> RPi/PC'
**Note:** Use '[pjon_grpc_clientserver.py](../../../clients/python/pjon_grpc_clientserver.py)' for receiving messages on RPi/PC side

Enable sending messages every 1 second from Arduino to RPi/PC:
```
$ ./pjon_grpc_client.py 1 21 M-1
Client received: M-1>1
```
On RPi/PC side we should see:
```
$ ./pjon_grpc_clientserver.py
Client-Server received: node_id=21, data=<Incoming message every 1 sec
Client-Server received: node_id=21, data=<Incoming message every 1 sec
...
```
