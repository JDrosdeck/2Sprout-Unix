import socket, sys, struct, time, md5

hostname = '127.0.0.1'
port = 4950
#msg = "5a0006c02db09b6cf269d50c7f39bb23^what^120209^0^<sproutCast><title>dump on your face</title></sproutCast>"
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	

for x in xrange(10):
	m = md5.new()
	tempMessage = "what^120510^" + str(x) + "^	<?xml version=\"1.0\" encoding=\"UTF-8\"?><page id=\"122\" update=\"0\"><url>http://lifehacker.com/5059400/extractnow-is-a-lightning-fast-bulk-extraction-tool</url><title>Featured Windows Download: ExtractNow Is a Lightning Fast Bulk Extraction Tool</title><timestamp>1223456554</timestamp></page>"
	m.update(tempMessage);
	msg = str(m.hexdigest()) + "^" + tempMessage
	print msg
	time.sleep(.01)
	s.sendto(msg,(host,port))

m = md5.new()
tempMessage = "what^120510^" + str(20) + "^	<?xml version=\"1.0\" encoding=\"UTF-8\"?><page id=\"122\" update=\"0\"><url>http://lifehacker.com/5059400/extractnow-is-a-lightning-fast-bulk-extraction-tool</url><title>Featured Windows Download: ExtractNow Is a Lightning Fast Bulk Extraction Tool</title><timestamp>1223456554</timestamp></page>"
m.update(tempMessage);
msg = str(m.hexdigest()) + "^" + tempMessage
print msg
time.sleep(.01)
s.sendto(msg,(host,port))



