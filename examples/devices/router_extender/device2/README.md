# Device #2

This example show how possible control Arduino PINs or send messages to RPi/PC via [router_2_bus](../router_2_bus)

This device connected to [router_2_bus](../router_2_bus) via [ThroughSerialAsync](https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughSerialAsync) strategy ([HC-12 module](http://statics3.seeedstudio.com/assets/file/bazaar/product/HC-12_english_datasheets.pdf))

## Examples

### Examples of request <-> response (RPi/PC <-> Arduino)

**Note:** from RPi/PC side uses python client: [pjon_grpc_client.py](../../../clients/python/pjon_grpc_client.py)

Read Hardware Digital PIN (_digitalRead(13)_):

```bash
$ ./pjon_grpc_client.py 1 51 H-13
Client received: H-13>0
```

Read Hardware Analog PIN (_analogRead(14)_):

```bash
$ ./pjon_grpc_client.py 1 51 H-14
Client received: H-14>275
```

Write Hardware Digital PIN (_digitalWrite(13, 1)_):

```bash
$ ./pjon_grpc_client.py 1 51 H-13-1
Client received: H-13-1>1
```

Read Virtual PIN:

```bash
$ ./pjon_grpc_client.py 1 51 V-0
Client received: V-0>0
```

Write Virtual PIN:

```bash
$ ./pjon_grpc_client.py 1 51 V-0-1
Client received: V-0-1>1
```

Read again the same PIN (after write):

```bash
$ ./pjon_grpc_client.py 1 51 V-0
Client received: V-0>1
```

Make test, number in response will be increased on 1:

```bash
$ for i in $(seq 1 2); do ./pjon_grpc_client.py 1 51 N; done
Client received: N>1
Client received: N>2
```

Wrong command:

```bash
$ ./pjon_grpc_client.py 1 51 T
Client received: T>wrong command
```

### Example of sending messages (Arduino -> RPi/PC)

**Note:** from RPi/PC side for receiving messages uses python script: [pjon_grpc_clientserver.py](../../../clients/python/pjon_grpc_clientserver.py)

Enable sending messages every 1 second from Arduino to RPi/PC:

```bash
$ ./pjon_grpc_client.py 1 51 M-1
Client received: M-1>1
```

On RPi/PC side we should see:

```bash
$ ./pjon_grpc_clientserver.py
Client-Server received: node_id=51, data=<Incoming message every 1 sec
Client-Server received: node_id=51, data=<Incoming message every 1 sec
...
```
