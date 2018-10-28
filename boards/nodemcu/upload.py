#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

# Print iterations progress
def printProgress (iteration, total, prefix = '', suffix = '', decimals = 2, barLength = 100):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : number of decimals in percent complete (Int)
        barLength   - Optional  : character length of bar (Int)
    """
    total = 1 if total == 0 else total
    filledLength    = int(round(barLength * iteration / float(total)))
    percents        = round(100.00 * (iteration / float(total)), decimals)
    bar             = '█' * filledLength + '-' * (barLength - filledLength)
    sys.stdout.write('\r%s |%s| %s%s %s' % (prefix, bar, percents, '%', suffix)),
    sys.stdout.flush()
    if iteration == total:
        sys.stdout.write('\n')
        sys.stdout.flush()

file = "mq.lua"
host = "mqtt.lan"
dest = "/dim/mac/u/"
destr = "/dim/mac/ur/"
import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe
import json


publish.single(dest, '{"payload": { "command":"open", "file":"%s"}}' % file, hostname=host)
print "Waiting for ESP at", dest
msg = subscribe.simple(destr, hostname=host)
print("%s %s" % (msg.topic, msg.payload))

i = 0
len = 0
with open(file, 'r') as f:
    for len, l in enumerate(f):
        pass
with open(file, 'r') as f:
  for line in f:
    printProgress(i,len,prefix = 'Progress', suffix = 'Complete', barLength=50)
    data = {}
    data["payload"] = {}
    data["payload"]["command"] = "l"
    data["payload"]["line"] = line
    publish.single(dest, json.dumps(data), hostname=host)
    msg = subscribe.simple(destr, hostname=host)
    #print("ur/ %s %s" % (msg.topic, msg.payload))
    i = i + 1

publish.single(dest, '{"payload":{"command": "close"}}', hostname=host)
publish.single(dest, '{"payload": { "command":"run", "file":"%s"}}' % file.replace('.lua', '.lc'), hostname=host)

print "\r\ndone uploading"
