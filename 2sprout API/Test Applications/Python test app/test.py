import threading
import sprout, time


class get2sproutFeed(threading.Thread):
	def run(self):
		print "WAHT"
		sprout.getFeed()
		

class get2sproutItem(threading.Thread):
	def run(self):
		print "HERE"
		while 1:
			data = sprout.getNextItem()
			print data


get2sproutFeed().start()
get2sproutItem().start()

time.sleep(1)
#get2sproutFeed().join()
#get2sproutItem().join()