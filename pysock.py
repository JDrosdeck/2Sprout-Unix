import socket, sys, struct, time, md5

hostname = '127.0.0.1'
port = 4950
#msg = "5a0006c02db09b6cf269d50c7f39bb23^what^120209^0^<sproutCast><title>dump on your face</title></sproutCast>"
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	

for x in xrange(50):
	m = md5.new()
	tempMessage = "what^120510^" + str(x) + "^<sproutCast><title> on your facedump oyouryouryouyouryyouryouryouryoyouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryoururyouryouryouryouryouryouryouryouryouryouryouryouryourouryouryoyouryouryouryouryouryouryouryouryouryoururyouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryourn your facedump on your facedump on your facedump on your facedump on your facedump on your facedump on your facen your facedump on your faceyouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryouryour</title></sproutCast>"
	m.update(tempMessage);
	msg = str(m.hexdigest()) + "^" + tempMessage
	print msg
	time.sleep(.01)
	s.sendto(msg,(host,port))




