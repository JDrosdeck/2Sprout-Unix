#!/usr/bin/env python
import Queue
import threading
import urllib2
import time, os, sys, pp
import BeautifulSoup 

hosts = ["http://yahoo.com", "http://google.com", "http://amazon.com",
        "http://ibm.com", "http://apple.com"]

queue = Queue.Queue()
out_queue = Queue.Queue()




class readFromPipe(threading.Thread):
	def run(self):
		if os.access('/tmp/GlobalQueue', os.F_OK) == False: #check to see if the named pipe exists already (os.F_OK) is used to check file existance
			os.mkfifo('/tmp/GlobalQueue') #file does not exists so make the quee
		while 1: #do this forever
			print "reading from Queue" #take this out
			fd = open('/tmp/GlobalQueue','r') # open the pipe for reading
			dataRead = fd.readline() #read the line from the named pipe
			#queue.put(dataRead) #put the data from the pipe into the queue
			queue.put(dataRead)
			


class ThreadUrl(threading.Thread):
    """Threaded Url Grab"""

    def run(self):
		ppservers = ()
		ncpus = 3
		job_server = pp.Server(ncpus, ppservers=ppservers)
		while 1:
			time.sleep(.01)
			if queue.empty() == False:
				host = queue.get()
				job1 = job_server.submit(getSiteHTML, (host,),(),("urllib2",))
				result = job1()
				print "recieved: ", result
				#place chunk into out queue
				out_queue.put(result)

def getSiteHTML(n):
	url = urllib2.urlopen(n)
	chunk = url.read()
	return chunk

def mineHTML(n):
	soup = BeautifulSoup.BeautifulSoup(n)
	title = soup.findAll(['title'])
	print title
	return title

class DatamineThread(threading.Thread):
    """Threaded Url Grab"""
    def run(self):
		ppservers = ()
		ncpus = 3
		job_server = pp.Server(ncpus, ppservers=ppservers)
		while 1:
			time.sleep(.01)
			if out_queue.empty() == False:
				#grabs host from queue
				chunk = out_queue.get()
				job2 = job_server.submit(mineHTML, (chunk,),(),("BeautifulSoup",))
				result2 = job2()
				job_server.print_stats()
				print result2



start = time.time()


def main():
	


#populate queue with data
	readFromPipe().start()


	
	ThreadUrl().start()

#	t.start()
					 			
	DatamineThread().start()

	
	
#	dt.start()


#wait on the queue until everything has been processed


main()
print "Elapsed Time: %s" % (time.time() - start)
