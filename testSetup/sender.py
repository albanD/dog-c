#!/usr/bin/python

from statsd import statsd, DogStatsd
from random import random

import time

statsd.connect('localhost', 8125)

def getRandTag(number):
	tags = []
	for i in range(0, number):
		tags += ['tag%s' % (int(random()*100))]
	return tags

while True:
	print('Sending metric')
	statsd.increment('page.views', tags=getRandTag(5), sample_rate=0.5)
	print('Sending gauge')
	with DogStatsd() as batch:
		batch.gauge('users.online', 123)
		batch.gauge('active.connections', 101)
	time.sleep(0.01)
