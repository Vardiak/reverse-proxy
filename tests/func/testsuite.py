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
    
    def test_close_during_answer(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        packet = self.socket.recv(10)
        self.socket.close()

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

    def test_close_before_reply(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        self.socket.close()

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

    def test_xoxo_in_body(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 10\r\n\r\nsalut\0\0sam"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        self.socket.close()

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')
    
    # def test_xoxo_in_request_line(self):
    #     self.socket.connect(("localhost", 80))
    #     req = "GET /\0index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
    #     self.socket.send(req.encode())
    #     response = recvall(self.socket, 2)
    #     res = re.match("HTTP\/1\.1 400[\s\S]+", response)
    #     self.assertIsNotNone(res)

    #     a = requests.get('http://localhost:8000')
    #     self.assertEqual(a.text, 'good\n')

    def test_xoxo_in_header_name(self):
        self.socket.connect(("localhost", 8000))
        req = "GET /\0index.html HTTP/1.1\r\nHos\0t: 127.0.0.1\r\n\r\n"
        self.socket.send(req.encode())
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 400[\s\S]+", response)
        self.assertIsNotNone(res)

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

    # def test_xoxo_in_bad_header_content(self):
    #     self.socket.connect(("localhost", 8000))
    #     req = "GET /\0index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nProut: dede\0salut\r\n\r\n"
    #     self.socket.send(req.encode())
    #     response = recvall(self.socket, 2)
    #     res = re.match("HTTP\/1\.1 400[\s\S]+", response)
    #     self.assertIsNotNone(res)

    #     a = requests.get('http://localhost:8000')
    #     self.assertEqual(a.text, 'good\n')

    # def test_xoxo_in_bad_header_content_2(self):
    #     self.socket.connect(("localhost", 8000))
    #     req = "GET /\0index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: clos\0e\r\n\r\n"
    #     self.socket.send(req.encode())
    #     response = recvall(self.socket, 2)
    #     res = re.match("HTTP\/1\.1 400[\s\S]+", response)
    #     self.assertIsNotNone(res)

    #     a = requests.get('http://localhost:8000')
    #     self.assertEqual(a.text, 'good\n')

class Timeout(unittest.TestCase):

    def setUp(self):
        self.out = subprocess.Popen("cd ../../ && ./spider ./tests/configs/config_timeouts.json &> /dev/null", shell=True, preexec_fn=os.setsid)
        time.sleep(0.2)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    def tearDown(self):
        self.socket.close()
        os.killpg(os.getpgid(self.out.pid), signal.SIGTERM)
        self.out.communicate()
    
    def test_transaction(self):
        self.socket.connect(("localhost", 8000))
        for i in range(11):
            self.socket.send("....".encode())
            time.sleep(0.5)
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 408[\s\S]+X-Timeout-Reason: Transaction[\s\S]+", response)
        self.assertIsNotNone(res)

    def test_keep_alive(self):
        self.socket.connect(("localhost", 8000))
        time.sleep(6)
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 408[\s\S]+X-Timeout-Reason: Keep-Alive[\s\S]+", response)
        self.assertIsNotNone(res)

    def test_throughput(self):
        self.socket.connect(("localhost", 8000))
        self.socket.send(".".encode())
        time.sleep(1.1)
        response = recvall(self.socket, 2)
        res = re.match("HTTP\/1\.1 408[\s\S]+X-Timeout-Reason: Throughput[\s\S]+", response)
        self.assertIsNotNone(res)

class ReverseProxyTimeout(unittest.TestCase):

    def setUp(self):
        self.cmd1 = "FLASK_APP=backend.py FLASK_RUN_PORT=8001 flask run &> /dev/null"
        self.backend1 = subprocess.Popen(self.cmd1, shell=True, preexec_fn=os.setsid)
        time.sleep(0.8)
        self.out = subprocess.Popen("cd ../../ && ./spider ./tests/configs/config_reverse_proxy_timeout.json 1 &> /dev/null", shell=True, preexec_fn=os.setsid)
        time.sleep(0.2)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def tearDown(self):
        self.socket.close()
        os.killpg(os.getpgid(self.out.pid), signal.SIGTERM)
        os.killpg(os.getpgid(self.backend1.pid), signal.SIGTERM)
        self.out.communicate()
        self.backend1.communicate()
    
    def test_fail_round_robin(self):
        a = requests.get('http://localhost:8000/timeout')
        self.assertEqual(a.status_code, 504)

class ReverseProxy(unittest.TestCase):

    def setUp(self):
        self.cmd1 = "FLASK_APP=backend.py FLASK_RUN_PORT=8001 flask run &> /dev/null"
        self.cmd2 = "FLASK_APP=backend.py FLASK_RUN_PORT=8002 flask run &> /dev/null"
        self.backend1 = subprocess.Popen(self.cmd1, shell=True, preexec_fn=os.setsid)
        self.backend2 = subprocess.Popen(self.cmd2, shell=True, preexec_fn=os.setsid)
        time.sleep(0.8)
        self.out = subprocess.Popen("cd ../../ && ./spider ./tests/configs/config_reverse_proxy_test.json 1 &> /dev/null", shell=True, preexec_fn=os.setsid)
        time.sleep(0.2)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def tearDown(self):
        self.socket.close()
        os.killpg(os.getpgid(self.out.pid), signal.SIGTERM)
        os.killpg(os.getpgid(self.backend1.pid), signal.SIGTERM)
        os.killpg(os.getpgid(self.backend2.pid), signal.SIGTERM)
        self.out.communicate()
        self.backend1.communicate()
        self.backend2.communicate()

    def test_round_robin(self):
        a = requests.get('http://localhost:8000')
        self.assertEqual(a.status_code, 200)
        self.assertEqual(a.text, 'good\n')

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good2\n')

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.text, 'good\n')

    def test_fail_round_robin(self):
        os.killpg(os.getpgid(self.backend1.pid), signal.SIGTERM)
        self.backend1.communicate()

        a = requests.get('http://localhost:8000')
        self.assertEqual(a.status_code, 502)

        self.backend1 = subprocess.Popen(self.cmd1, shell=True, preexec_fn=os.setsid)

    def test_fail_over(self):
        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost2'})
        self.assertEqual(a.status_code, 200)
        self.assertEqual(a.text, 'good\n')

        os.killpg(os.getpgid(self.backend1.pid), signal.SIGTERM)
        self.backend1.communicate()

        time.sleep(1)

        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost2'})
        self.assertEqual(a.text, 'good2\n')

        self.backend1 = subprocess.Popen(self.cmd1, shell=True, preexec_fn=os.setsid)
        
    def test_fail_robin(self):
        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost3'})
        self.assertEqual(a.status_code, 200)
        self.assertEqual(a.text, 'good\n')

        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost3'})
        self.assertEqual(a.text, 'good\n')

        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost3'})
        self.assertEqual(a.text, 'good2\n')

        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost3'})
        self.assertEqual(a.text, 'good\n')

        os.killpg(os.getpgid(self.backend1.pid), signal.SIGTERM)
        self.backend1.communicate()

        time.sleep(1)

        a = requests.get('http://127.0.0.1:8000', headers={'Host': 'localhost3'})
        self.assertEqual(a.text, 'good2\n')

        self.backend1 = subprocess.Popen(self.cmd1, shell=True, preexec_fn=os.setsid)

    def test_forwarded(self):
        a = requests.get('http://localhost:8000/forwarded')
        self.assertEqual(a.text, 'for=127.0.0.1;host=localhost:8000;proto=http')




if __name__ == '__main__':
    unittest.main()
