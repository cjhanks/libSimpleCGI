import bottle

app = bottle.Bottle()


@app.route('/hello/<name>/python', method=['GET'])
def hello(name):
    return 'Hello Python {}'.format(name)

print('Python loaded')
