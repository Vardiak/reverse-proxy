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

class TestRequests(unittest.TestCase):
     
    # def test_status_code(self):

    #     os.system('./spider tests/configs/config_subject.json')
    #     response = requests.get('http://127.0.0.1:8000/index.html')
    #     status_code = response.status_code
    #     self.assertEqual(status_code, 200)
    #     serial.write('\x03')

    
    def test_socket_get_response(self):
        # out = subprocess.Popen("../../spider ../configs/config_subject.json", shell=True, preexec_fn=os.setsid)

        # time.sleep(1)

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("localhost", 8000))
        req = "GETindex.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
        s.send(req.encode())
        response = s.recv(1024).decode()
        print(response)
        url = re.search("(?:https?://[a-zA-Z0-9.]+(?::[0-9]*)?)?(/.*)", response)
        headers = re.findall("(\\S+):[ \t]*(\\S+)[ \t]*", response)
        #[ERROR] IO check failed ! Expected to match `HTTP\/1\.1 200[\s\S]+

        print(url)
        print(headers)

        s.close()


        # os.killpg(os.getpgid(out.pid), signal.SIGTERM)
        # out.communicate()




if __name__ == '__main__':
    unittest.main()
