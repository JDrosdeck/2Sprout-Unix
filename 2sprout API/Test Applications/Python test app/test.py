import threading
import sprout


class get2sproutFeed(threading.Thread):
	def run(self):
		sprout.getFeed()
		

class get2sproutItem(threading.Thread):
	def run(self):
		data = sprout.getNextItem()
		print data
		

sprout.startFeed()
get2sproutFeed().start()
get2sproutItem().start()

sleep(1)
get2sproutFeed().join()
get2sproutItem().join()