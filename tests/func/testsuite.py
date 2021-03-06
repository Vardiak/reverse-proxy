import os
import sys
import re
# import requests
# import serial
import unittest
import socket
import subprocess
import signal
import time

def recvall(sock):
    BUFF_SIZE = 4096
    data = bytearray()
    while True:
        packet = sock.recv(BUFF_SIZE)
        if not packet:
            break
        data.extend(packet)
    return data.decode()

class TestRequests(unittest.TestCase):

    def setUp(self):
        self.out = subprocess.Popen("cd ../../ && ./spider ./tests/configs/config_basic.json &> /dev/null", shell=True, preexec_fn=os.setsid)
        time.sleep(0.1)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def tearDown(self):
        self.socket.close()
        os.killpg(os.getpgid(self.out.pid), signal.SIGTERM)
        self.out.communicate()
     
    # def test_status_code(self):

    #     os.system('./spider tests/configs/config_subject.json')
    #     response = requests.get('http://127.0.0.1:8000/index.html')
    #     status_code = response.status_code
    #     self.assertEqual(status_code, 200)
    #     serial.write('\x03')

    
    def test_basic_request(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)

        self.assertIsNotNone(res)

    def test_hostname(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket)
        res = re.match("HTTP\/1\.1 200[\s\S]+\r\n\r\ngood\n", response)

        self.assertIsNotNone(res)


if __name__ == '__main__':
    unittest.main()
