#!/usr/bin/env python

from concurrent import futures
import time
import datetime

import grpc
import pjongrpc_pb2
import pjongrpc_pb2_grpc
import threading

_ONE_DAY_IN_SECONDS = 60 * 60 * 24


class Arduino(pjongrpc_pb2_grpc.ArduinoServicer):
    def RPiArduino(self, request, context):
        print("%s: Client-Server received: node_id=%d, data=%s" % (datetime.datetime.now(), request.node_id, request.data))
        return pjongrpc_pb2.Arduino_Reply(message='done')

def serv1():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    pjongrpc_pb2_grpc.add_ArduinoServicer_to_server(Arduino(), server)
    server.add_insecure_port('[::]:50061')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)

def serv2():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    pjongrpc_pb2_grpc.add_ArduinoServicer_to_server(Arduino(), server)
    server.add_insecure_port('[::]:50062')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    s1 = threading.Thread(target=serv1)
    s1.daemon = True
    s1.start()
    s2 = threading.Thread(target=serv2)
    s2.daemon = True
    s2.start()
    while True:
        time.sleep(1)
