import socket, sys, struct, time, md5
from itertools import izip, cycle
import base64, random



def XOR(value,key):
	return ''.join(chr(ord(x) ^ ord(y)) for (x,y) in izip(value, cycle(key)))

# "HZmCniVvOX"
# "kwYCZyfuTI"

hostname = '127.0.0.1'
port = 4950
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	

word = 'enkeExxZIyJ6eWxWBTUTAUFmUxdKfUJgGh0rLVkOZA9+MRY+CQsFPDgldzoxBFxz'
word = base64.b64decode(word)
print word
word = XOR(word, '2#sPr0uT5!$')
print word

'''
for x in xrange(10):
	m = md5.new()
	tempMessage = "what^120511^"+ str(x) + "^0^TESTdddkjESTES125.99</price><spdroutcast><url>www.amazon.com/prod1d=43453</url><title>gold watch</title></sproutcast><price>125.99</price>kjg;gkhvlhvjhvkhgadfadlhfhgckghhckfugotdufot"
	m.update(tempMessage)
	msg1 = str(m.hexdigest()) + "^" + tempMessage
	print msg1
	msg = "81SAy8gW" + msg1
	msg = XOR(msg,'mMoeygLQih')
	msg = base64.b64encode(msg)
	#g = random.uniform(.002)
	time.sleep(.0000002)
	#print g
	s.sendto(msg,(host,port))
	del msg1
	del msg
	del m
	del tempMessage
'''


