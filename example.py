import time
from pthread import Thread

def do_stuff(value):
    return f"result: {value}"

threads = [ Thread(func=do_stuff, args=(x,)) for x in range(10) ]

for thread in threads:
    result = thread.join()
    print(result)
