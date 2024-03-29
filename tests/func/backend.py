import flask
import os
import time

app = flask.Flask(__name__)

@app.route('/')
def hello_world():
    if int(os.environ['FLASK_RUN_PORT']) == 8001:
        return 'good\n'
    return 'good2\n'

@app.route('/forwarded')
def forwarded():
    return str(flask.request.headers['Forwarded'])

@app.route('/hop_by_hop')
def hop_by_hop():
    return str(flask.request.headers)

@app.route('/ip')
def ip():
    return str(flask.request.remote_addr)

@app.route('/timeout')
def timeout():
    time.sleep(1.1)
    return str('did it work?')

@app.route('/method', methods=["DELETE"])
def method():
    return str('did it work?')
