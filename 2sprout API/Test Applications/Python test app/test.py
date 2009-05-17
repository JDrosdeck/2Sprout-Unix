import threading
import sprout, time


class get2sproutFeed(threading.Thread):
	def run(self):
		sprout.getFeed()
		x = 0
		while 1:
			data = sprout.getSproutItem()
			x = x + 1
			print data
			print x



get2sproutFeed().start()
