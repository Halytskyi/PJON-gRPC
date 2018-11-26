### Install gRPC and gRPC tools:
```
$ virtualenv .venv
$ source .venv/bin/activate
$ python -m pip install --upgrade pip
$ python -m pip install grpcio
$ python -m pip install grpcio-tools
```

### Generate gRPC code:
```
$ ./run_codegen.py
```
OR
```
$ python -m grpc_tools.protoc -Iprotos --python_out=. --grpc_python_out=. protos/pjongrpc.proto
```

### Examples of run
**pjon_grpc_client.py** - client for sending requests to devices and getting responses
```
$ ./pjon_grpc_client.py 1 11 n
Client received: n>1
```
where:
```
1 - master ID
11 - device ID
n - command
```
**pjon_grpc_clientserver.py** - client-server script for receiving messages from devices (works in daemon mode)
```
$ ./pjon_grpc_clientserver.py
Client-Server received: node_id=11, data=29
```
