#!/usr/bin/env python

from __future__ import print_function
import sys

import grpc
import pjongrpc_pb2
import pjongrpc_pb2_grpc


def run():
    if sys.argv[1] == "1":
        pjon_server_address = 'localhost:50051'
    elif sys.argv[1] == "2":
        pjon_server_address = 'localhost:50052'
    else:
        print("Wrong 'Master ID'")
        sys.exit(1)
    channel = grpc.insecure_channel(pjon_server_address)
    stub = pjongrpc_pb2_grpc.ArduinoStub(channel)
    response = stub.RPiArduino(pjongrpc_pb2.Arduino_Request(node_id=int(sys.argv[2]), data=sys.argv[3]))
    print("Client received: " + response.message)


if __name__ == '__main__':
    run()
