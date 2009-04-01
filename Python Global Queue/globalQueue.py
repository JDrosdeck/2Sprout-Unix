#!/usr/bin/env python
import Queue
import threading
import urllib2, re
import time, os, sys, pp, md5, datetime
import BeautifulSoup 



queue = Queue.Queue()
out_queue = Queue.Queue()
finished_queue = Queue.Queue()



class writeToCaster(threading.Thread):
	def run(self):
		if os.access('/tmp/sproutLocalIPC', os.F_OK) == False:
			os.mkfifo('/tmp/sproutLocalIPC')
		while 1:
			if finished_queue.empty() == False:
				print "reading finialized data from Queue"
				finalizedString = finished_queue.get()
				print "Pre finalizedString = ", finalizedString
				m = md5.new()
				timeNow = datetime.datetime.now().strftime("%d%m%Y")
				print "the Date is: ", timeNow
				endString = "what^"+str(timeNow)+"^"+str(finalizedString)
				m.update(endString)
				finalMsg = str(m.hexdigest()) + "^" + endString
				print "FINAL MESSAGE", finalMsg
				#fileDes = open('/tmp/sproutLocalIPC','w')
				#fileDes.write(finalMsg)
				#fileDes.close()
			else:
				time.sleep(.001)
				
			


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
			fd.close()
			


class ThreadUrl(threading.Thread):
    """Threaded Url Grab"""

    def run(self):
		ppservers = ()
		ncpus = 1
		job_server = pp.Server(ncpus, ppservers=ppservers)
		while 1:
			time.sleep(.01)
			if queue.empty() == False:
				print "WERWER"
				host = queue.get()
				job1 = job_server.submit(getSiteHTML, (host,),(),("urllib2",))
				result = job1()
				#print "recieved: ", result
				#place chunk into out queue
				out_queue.put(result)

def getSiteHTML(n):
	url = urllib2.urlopen(n)
	chunk = url.read()
	return chunk

def mineHTML(n):
	#soup = BeautifulSoup.BeautifulSoup(n)
	#title = soup.findAll(['title'])
	schema	= open('quark.def','r').read()
	regex	= re.compile("\[%(.*)%\].*?\n?\s*{\s*(.*)")
	matches = re.findall(regex, schema)
	
	for key, value in matches:
		regex	= re.compile("<##DATA##>")
		pattern	= regex.sub('(.*)',value)
		regex	= re.compile(pattern)
		matches = re.findall(regex, n)
		print key, matches
	
	#print title
	#return title

class DatamineThread(threading.Thread):
    """Threaded Url Grab"""
    def run(self):
		print "HERE"
		ppservers = ()
		ncpus = 2
		job_server = pp.Server(ncpus, ppservers=ppservers)
		while 1:
			time.sleep(.01)
			if out_queue.empty() == False:
				#grabs host from queue
				print "WHAT"
				chunk = out_queue.get()
				job2 = job_server.submit(mineHTML, (chunk,),(),("re",))
				result2 = job2()
				job_server.print_stats()
				#print result2
				finished_queue.put(result2)



start = time.time()


def main():
	
	readFromPipe().start()
	ThreadUrl().start()					 			
	DatamineThread().start()
	writeToCaster().start()


#wait on the queue until everything has been processed


main()
print "Elapsed Time: %s" % (time.time() - start)
