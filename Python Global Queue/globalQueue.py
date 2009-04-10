#!/usr/bin/env python
import Queue
import threading
import urllib2, re
import time, os, sys, pp, md5, datetime
import BeautifulSoup 



urlData = Queue.Queue(0)

			
			


"""
readFromPipeAndMine:

	This function will read from the named pipe from CGlobalQueue, the url that it reads
	will then be fetched. The data that it reads will then be used passed off to parrall python
	for definition file mining
"""
class readFromPipeAndMine(threading.Thread):
	def run(self):
		if os.access('/tmp/GlobalQueue', os.F_OK) == False: #check to see if the named pipe exists already (os.F_OK) is used to check file existance
			os.mkfifo('/tmp/GlobalQueue') #file does not exists so make the quee
		while 1: 
			print "reading from Queue" 
			fd = open('/tmp/CqueueOutgoing','r') # open the pipe for reading
			dataRead = fd.readline() #read the line from the named pipe
			#queue.put(dataRead) #put the data from the pipe into the queue
			fd.close()
			siteData = getSiteHTML(dataRead)
			urlData.put(siteData) # put the html fetched into a queue



class mineData(threading.Thread):
	def run(self):
		ppservers = ()
		ncpus = 2
		job_server = pp.Server(ncpus, ppservers=ppservers)
		while 1:
			if(!urlData.empty()):
				htmlToMine = urlData.get()
				jobs.append(job_server.submit(mineHTML,(htmlToMine),(),()))
				#retried the results of all the submitted jobs
				for job in jobs:
					result = job()
					
					#we should then write the result to the caster application and to the database.
					
					
					if result:
						break



def mineHTML(n):
	return n




		


def getSiteHTML(n):
	url = urllib2.urlopen(n)
	chunk = url.read()
	return chunk








def main():
	
	for x in xrange(10):
		readFromPipeAndMine().start()

	mineData().start()

#wait on the queue until everything has been processed


main()
print "Elapsed Time: %s" % (time.time() - start)
