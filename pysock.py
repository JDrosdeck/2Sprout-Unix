import socket, sys, struct, time, md5
from itertools import izip, cycle
import base64



def XOR(value,key):
	return ''.join(chr(ord(x) ^ ord(y)) for (x,y) in izip(value, cycle(key)))



hostname = '127.0.0.1'
port = 4950
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	

for x in xrange(100):
	m = md5.new()
	tempMessage = "what^120511^"+ str(x) + "^0^dwatch</title></sproutcast><price>125.99</price><sproutcast><url>www.amazon.com/prod1d=43453</url><title>gold watch</title></sproutcast><price>125.99</price>"
	m.update(tempMessage)
	msg1 = str(m.hexdigest()) + "^" + tempMessage
	print msg1
	msg = "xfEdW2Ca" + msg1
	msg = XOR(msg,'pb0WDljayt')
	msg = base64.b64encode(msg)
	time.sleep(.10)
	s.sendto(msg,(host,port))



