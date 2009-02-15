import threading,os,Queue,time,HTMLParser, string, re
#i'm not sure about the following items
import socket, sys, struct, time
from xml.dom.minidom import Document
from BeautifulSoup import BeautifulSoup
from htmlentitydefs import name2codepoint as n2cp
from urllib import urlopen





#The Add New Data function will read from the named pipe, and anything read in will be placed into the global queue
class addNewData(threading.Thread):

	def run(self):
		if os.access('/tmp/GlobalQueue', os.F_OK) == False: #check to see if the named pipe exists already (os.F_OK) is used to check file existance
			os.mkfifo('/tmp/GlobalQueue') #file does not exists so make the quee
		while 1: #do this forever
			print "reading from Queue" #take this out
			fd = open('/tmp/GlobalQueue','r') # open the pipe for reading
			dataRead = fd.readline() #read the line from the named pipe
			lock.acquire(1) #aquire the global lock so we can have sole access of the shared queue
			#queue.put(dataRead) #put the data from the pipe into the queue
			queue.append(dataRead)
			print "before " 
			size = len(queue)
			print size 
			lock.release() #release the lock as soon as possible..right now
		

#the grabShitFromWeb function gets the next item off the queue and just prints it out for now
class grabShitFromWeb(threading.Thread):
	def run(self):
		print "grabbing shit from web"
		while 1: 
			lock.acquire(1) #aquire a lock for the shared queue
			if len(queue) <= 0: #if the queue is empty
				lock.release() 
				time.sleep(0.01) #(sleep)may need to make this longer its takes about 25% of the cpu
			else:
				url = queue[0] 
				queue.pop(0)
				lock.release() 
				
				#this is where all the cool stuff happens




				if pageFunctions().validateURL(url):

			      # append http:// to URL if needed 
			    		if url.startswith("http://") != True:  
							url = "http://" + url
							msg = 'Page Exist'
							# get page html
				         # get page html
				        page_html = urlopen(url).read()
				        page_html = page_html.lower()

				         # create instance of BeautifulSoup for data extraction
				        BSoup = BeautifulSoup(page_html)

				         # title
				        title = BSoup.html.head.title.string

				         # description
				        description = BSoup.head.find("meta",{"name":"description"})
				        if description and 'content' in dict(description.attrs):
				           description = description['content']

				         # keyword
				        keywords = BSoup.head.find("meta",{"name":"keywords"})
				        if keywords and 'content' in dict(keywords.attrs):
				           keywords = keywords['content']         

				        # medium
				        medium = BSoup.head.find("meta",{"name":"medium"})
				        if medium and 'content' in dict(medium.attrs):
				           medium = medium['content']

				        doc = Document()  

				        sproutcast = doc.createElement("sproutcast")
				        doc.appendChild(sproutcast)

				        urlNode = doc.createElement("url")
				        sproutcast.appendChild(urlNode)

				        urlData = doc.createTextNode(url)
				        urlNode.appendChild(urlData)    

				        titleNode = doc.createElement("title")
				        sproutcast.appendChild(titleNode)

				        titleData = doc.createTextNode(title)
				        titleNode.appendChild(titleData)

				        if description != None:
				           descriptionNode = doc.createElement("description")
				           sproutcast.appendChild(descriptionNode)

				           descriptionData = doc.createTextNode(description)
				           descriptionNode.appendChild(descriptionData)

				        if keywords != None:
				           keywordsNode = doc.createElement("keywords")
				           sproutcast.appendChild(keywordsNode)

				           keywordsData = doc.createTextNode(keywords)
				           keywordsNode.appendChild(keywordsData) 

				        if medium != None:
				           mediumNode = doc.createElement("medium")
				           sproutcast.appendChild(mediumNode)

				           mediumData = doc.createTextNode(medium)
				           mediumNode.appendChild(mediumData) 

				         #msg = keywords
				        msg = doc.toxml()
					print msg
			        
					# save general info to database
			         #dbPage = page(
			         #page_url          = url, 
			         #page_title        = title,
			         #page_date_update  = datetime.now(), 
			         #page_checksum     = page_hash,
			         #page_reindex      = 1)
			         #dbPage.save()
				#	print page_content
			
					
					
				
				
class pageFunctions():
	def validateURL(self,url):
	  urlRegEx = "([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}|(((news|telnet|nttp|file|http|ftp|https)://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+)(:[0-9]*)?[-A-Za-z0-9_\\$\\.\\+\\!\\*\\(\\),;:@&=\\?/~\\#\\%]*[^]'\\.}>\\),\\\"]"

	  if re.match(urlRegEx, url) != None:
	    return True
	  else:
	    return False

	def stripHTMLtags(self,html):
	  html = re.sub(r'(?is)<script.*?</script>', ' ', html)
	  html = re.sub(r'(?is)<style.*?</style>', ' ', html)
	  html = re.sub(r'(?is)<a.*?</a>', ' ', html)
	  html = re.sub(r'(?is)<.*?>', ' ', html)
	  html = re.sub(r'(?is)[^a-zA-Z]+', ' ', html)
	  return html

	def generateHash(self,input): 
	    import hashlib 
	    h = hashlib.sha1() 
	    h.update(input)
	    hash = h.hexdigest()
	    return hash


	def common_word_check(checkword):
	    return bool(common_words.objects.filter(word=checkword).count())
	
	
		
		
lock = threading.Lock() #set up a new global lock to protect the shared queue
#queue = Queue.Queue(0) #Queue.Queue(0) makes a new queue. 0 represents that the number of queue items is infinite
queue = [] #queue is actually a list
addNewData().start() #start this thread

for x in xrange(25):
	grabShitFromWeb().start() #start this thread

