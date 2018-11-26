### gRPC server-client for PJON bus

#### Install gRPC
```
$ sudo apt-get install build-essential autoconf libtool
$ mkdir /opt/libraries && cd /opt/libraries
$ git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
$ cd grpc && git submodule update --init
$ make
$ make install
$ git submodule update --init
$ cd third_party/protobuf
$ ./autogen.sh
$ ./configure
$ make
$ make install
```

#### PJON library
```
$ cd /opt/libraries
$ git clone https://github.com/gioblu/PJON.git
```

#### Compile
```
$ make
```

#### Configure
See example of config file with description: [conf/pjon-grpc.cfg](conf/pjon-grpc.cfg)

#### Execute
```
$ ./pjon_grpc_server
```
