import threading
import sprout


class get2sproutFeed(threading.Thread):
	def run(self):
		sprout.getFeed()
		

class get2sproutItem(threading.Thread):
	def run(self):
		data = sprout.getNextItem()
		print data
		

sprout.startFeed(23423)
get2sproutFeed().start()
get2sproutItem().start()