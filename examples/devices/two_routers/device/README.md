# Device #1

This example show how possible control Arduino PINs (transmit-receive) via [router_RxTx_busA](../router_RxTx_busA) or send messages to RPi/PC via [router_Tx_busB](../router_Tx_busB)

This device connected to [router_RxTx_busA](../router_RxTx_busA) and [router_Tx_busB](../router_Tx_busB) via [SoftwareBitBang](https://github.com/gioblu/PJON/tree/master/src/strategies/SoftwareBitBang) strategies (see [diagram](../images/PJON-gRPC_two_routers.jpg))

## Examples

### Examples of request <-> response (RPi/PC <-> Arduino)

**Note:** from RPi/PC side uses python client: [pjon_grpc_client.py](../../../clients/python/pjon_grpc_client.py)

Read Hardware Digital PIN (_digitalRead(13)_):

```bash
$ ./pjon_grpc_client.py 1 21 H-13
Client received: H-13>0
```

Write Hardware Digital PIN (_digitalWrite(13, 1)_):

```bash
$ ./pjon_grpc_client.py 1 21 H-13=1
Client received: H-13=1>ok
```

Read again the same PIN (after write):

```bash
$ ./pjon_grpc_client.py 1 21 H-13
Client received: H-13>1
```

Make test, number in response will be increased on 1:

```bash
$ for i in $(seq 1 2); do ./pjon_grpc_client.py 1 21 N; done
Client received: N>1
Client received: N>2
```

Wrong command:

```bash
$ ./pjon_grpc_client.py 1 21 T
Client received: T>err
```

### Example of sending messages (Arduino -> RPi/PC)

**Note:** from RPi/PC side for receiving messages uses python script: [pjon_grpc_clientserver.py](../../../clients/python/pjon_grpc_clientserver.py)

Enable sending messages every 1 second from Arduino to RPi/PC:

```bash
$ ./pjon_grpc_client.py 1 21 M=1
Client received: M=1>ok
```

On RPi/PC side we should see:

```bash
$ ./pjon_grpc_clientserver.py
2020-11-07 12:35:35.085915: Client-Server received: node_id=21, data=M<1 sec msg: 1
2020-11-07 12:35:36.090334: Client-Server received: node_id=21, data=M<1 sec msg: 2
2020-11-07 12:35:37.195941: Client-Server received: node_id=21, data=M<1 sec msg: 3
...
```

Enable sending messages every 0.2 second from Arduino to RPi/PC:

```bash
$ ./pjon_grpc_client.py 1 21 F=1
Client received: F=1>ok
```

On RPi/PC side we should see:

```bash
$ ./pjon_grpc_clientserver.py
2020-11-07 12:39:06.034376: Client-Server received: node_id=21, data=F<0.2 sec msg: 1
2020-11-07 12:39:06.240993: Client-Server received: node_id=21, data=F<0.2 sec msg: 2
2020-11-07 12:39:06.443820: Client-Server received: node_id=21, data=F<0.2 sec msg: 3
...
```
