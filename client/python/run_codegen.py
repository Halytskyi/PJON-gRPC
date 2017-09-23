#!/usr/bin/env python

"""Generates protocol messages and gRPC stubs."""

from grpc_tools import protoc

protoc.main(
    (
        '',
        '-Iprotos',
        '--python_out=.',
        '--grpc_python_out=.',
        'protos/pjongrpc.proto',
    )
)
