import threading
import sprout, time


class get2sproutFeed(threading.Thread):
	def run(self):
		while 1:
			data = sprout.getSproutItem()
			print data.upper()



get2sproutFeed().start()
