from Tkinter import *
import threading
import sprout, time, Queue

queue = Queue.Queue(0)




		



class get2sproutFeed(threading.Thread):
	def run(self):
		sprout.getFeed()
		

class get2sproutItem(threading.Thread):
	def run(self):
		data = sprout.getNextItem()
		queue.put(data)
		print data
		

root = Tk()
w = Label(root, text="Hello, world!")
w.pack()
get2sproutFeed().start()
get2sproutItem().start()

root.mainloop()




get2sproutFeed().join()
get2sproutItem().join()
