### Files:
**pjon_grpc_client.py** - client for sending requests to devices and getting responses
```
$ ./pjon_grpc_client.py 11 T:2:1
Client received: 1
```
**pjon_grpc_clientserver.py** - client-server script for receiving messages from devices (works in daemon mode)
```
$ ./pjon_grpc_clientserver.py
Client-Server received: node_id=11, data=29
```
**run_codegen.py** - generate gRPC code
```
$ ./run_codegen.py
```

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
$ python -m grpc_tools.protoc -Iprotos --python_out=. --grpc_python_out=. protos/pjongrpc.proto
```
OR
```
$ ./run_codegen.py
```
