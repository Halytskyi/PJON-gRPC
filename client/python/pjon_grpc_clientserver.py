#!/usr/bin/env python

from concurrent import futures
import time

import grpc
import pjongrpc_pb2
import pjongrpc_pb2_grpc

_ONE_DAY_IN_SECONDS = 60 * 60 * 24


class Arduino(pjongrpc_pb2_grpc.ArduinoServicer):
    def RPiArduino(self, request, context):
        print("Client-Server received: node_id=%d, data=%s" % (request.node_id, request.data))
        return pjongrpc_pb2.Arduino_Reply(message='done')

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    pjongrpc_pb2_grpc.add_ArduinoServicer_to_server(Arduino(), server)
    server.add_insecure_port('[::]:50052')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    serve()
