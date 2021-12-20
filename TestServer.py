#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import socket

HOST = '0.0.0.0'
PORT = 8090

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(5)

print('server start at: %s:%s' % (HOST, PORT))
print('wait for connection...')

while True:
    conn, addr = s.accept()
    print('connected by ' + str(addr))

    while True:
        indata = conn.recv(1024)
        if len(indata) == 0: # connection closed
            conn.close()
            print('client closed connection.')
            break
        print('recv: ' + indata.decode())

        outdata = 'echo ' + indata.decode()
        conn.send(outdata.encode())

    print('try to send to client ' + addr[0])
    s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s2.connect((addr[0], PORT))
    s2.send('thanks for connection'.encode())
    indata = s2.recv(1024)
    if len(indata) == 0: # connection closed
        s2.close()
        print('server closed connection.')
        break
    print('recv: ' + indata.decode())
