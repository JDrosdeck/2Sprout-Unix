from multiprocessing import Process 
import sprout, time


def getItem():
	while 1:
		json = sprout.getSproutItem()
		print json
	
p = Process(target=getItem, args=(,))
p.start()
p.join()