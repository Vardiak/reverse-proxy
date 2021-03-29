import flask
import os

app = flask.Flask(__name__)

@app.route('/')
def hello_world():
    if int(os.environ['FLASK_RUN_PORT']) == 8001:
        return 'good\n'
    return 'good2\n'

@app.route('/forwarded')
def forwarded():
    return str(flask.request.headers['Forwarded'])


@app.route('/ip')
def ip():
    return str(flask.request.remote_addr)
