import htmllib
import formatter
import string
import urllib, urlparse
import pycurl

class myParser(htmllib.HTMLParser):

    def __init__(self, base):
        htmllib.HTMLParser.__init__(self, formatter.NullFormatter())
        self.anchors = []
        self.base = base

    def anchor_bgn(self, href, name, type):
        self.save_bgn()
        if self.base:
            self.anchor = urlparse.urljoin(self.base, href)
        else:
            self.anchor = href

    def anchor_end(self):
        text = string.strip(self.save_end())
        if self.anchor and text:
            self.anchors.append((self.anchor))
            
            
 




if __name__ == '__main__':

	t = open('sites.txt')
	for line in t:
			print line
			URL = line
	    		f = urllib.urlopen(URL)
	    		p = myParser(URL)
    			p.feed(f.read())
    			p.close()
        		print len(p.anchors)
	    		print p.anchors
    			number1 = 0
    			for number in p.anchors:
    					print p.anchors[number1]
    					pycurl.announce(p.anchors[number1]) 	
					number1 = number1 + 1

