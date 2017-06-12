import bottle

app = bottle.Bottle()


@app.route('/python/hello/<name>', method=['GET'])
def hello(name):
    return 'Hello {}'.format(name)

print('Python loaded')
