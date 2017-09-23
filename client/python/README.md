Install gRPC and gRPC tools:
```
virtualenv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install grpcio
python -m pip install grpcio-tools
```

Generate gRPC code:
```
python -m grpc_tools.protoc -Iprotos --python_out=. --grpc_python_out=. protos/pjongrpc.proto
```
OR
```
./run_codegen.py
```
