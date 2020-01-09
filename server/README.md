# gRPC server-client for PJON bus

## Installation

### Install WiringPi

```bash
apt-get install wiringpi -y
```

### Install gRPC

#### From packages

```bash
apt-get install protobuf-compiler-grpc libgrpc++-dev -y
```

**Note:** tested with versions:

```bash
# dpkg -l | egrep "protobuf|libgrpc|wiringpi"
ii  libgrpc++-dev:armhf            1.16.1-1                            armhf        high performance general RPC framework (development)
ii  libgrpc++1:armhf               1.16.1-1                            armhf        high performance general RPC framework
ii  libgrpc-dev:armhf              1.16.1-1                            armhf        high performance general RPC framework (development)
ii  libgrpc6:armhf                 1.16.1-1                            armhf        high performance general RPC framework
ii  libprotobuf-c1:armhf           1.3.1-1+b1                          armhf        Protocol Buffers C shared library (protobuf-c)
ii  libprotobuf-dev:armhf          3.6.1.3-2+rpi1                      armhf        protocol buffers C++ library (development files) and proto files
ii  libprotobuf-lite17:armhf       3.6.1.3-2+rpi1                      armhf        protocol buffers C++ library (lite version)
ii  libprotobuf17:armhf            3.6.1.3-2+rpi1                      armhf        protocol buffers C++ library
ii  protobuf-compiler              3.6.1.3-2+rpi1                      armhf        compiler for protocol buffer definition files
ii  protobuf-compiler-grpc         1.16.1-1                            armhf        high performance general RPC framework - protobuf plugin
ii  wiringpi                       2.50                                armhf        The wiringPi libraries, headers and gpio command
```

#### From sources

**Note:** not working on OS Raspbian Buster on 01.01.2020 date due to compile errors

```bash
apt-get install build-essential autoconf libtool pkg-config libgflags-dev libgtest-dev clang-5.0 libc++-dev -y
mkdir /opt/libraries && cd /opt/libraries
git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc && git submodule update --init
make
make install
cd third_party/protobuf
./autogen.sh
./configure
make
make install
```

### PJON library

```bash
cd /opt/libraries
git clone https://github.com/gioblu/PJON.git
git checkout <release_version>
```

### Compile

```bash
make
```

## Configuration

See example of config file with description: [conf/pjon-grpc.cfg](conf/pjon-grpc.cfg)

## Execution

```bash
./pjon_grpc_server
```
