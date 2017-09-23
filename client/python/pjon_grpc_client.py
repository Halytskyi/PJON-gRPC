#!/usr/bin/env python

from __future__ import print_function
import sys

import grpc
import pjongrpc_pb2
import pjongrpc_pb2_grpc


def run():
    channel = grpc.insecure_channel('localhost:50051')
    stub = pjongrpc_pb2_grpc.ArduinoStub(channel)
    response = stub.RPiArduino(pjongrpc_pb2.Arduino_Request(node_id=int(sys.argv[1]), data=sys.argv[2]))
    print("Client received: " + response.message)


if __name__ == '__main__':
    run()
