import os
import re
import requests
import unittest
import socket
import subprocess
import signal
import time

def recvall(sock, m = 9999):
    BUFF_SIZE = 4096
    data = bytearray()
    i = 0
    while i < m:
        packet = sock.recv(BUFF_SIZE)
        if not packet:
            break
        data.extend(packet)
        i += 1
    return data.decode()

class TestRequests(unittest.TestCase):

    def setUp(self):
        self.out = subprocess.Popen("cd ../../ && ./spider ./tests/configs/config_basic.json &> /dev/null", shell=True, preexec_fn=os.setsid)
        time.sleep(0.2)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def tearDown(self):
        self.socket.close()
        os.killpg(os.getpgid(self.out.pid), signal.SIGTERM)
        self.out.communicate()
    
    def test_basic_request(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)

    def test_hostname(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)

        self.assertIsNotNone(res)

    def test_requests(self):
        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

    def test_default_vhost(self):
        a = requests.get('http://localhost:8000', headers={'Host': 'example.com'})
        self.assertEqual(a.text, 'good\n')

    def test_repeat_request(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)

        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)

    def test_repeat_default(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)

        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)
    
    def test_repeat_close_ambiguous(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: salut, close, keep-alive\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)
        self.assertIsNotNone(res)

        try:
            self.socket.send("test".encode())
            self.assertTrue(False)
        except:
            pass

if __name__ == '__main__':
    unittest.main()
