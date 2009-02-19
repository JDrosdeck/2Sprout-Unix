import socket, sys, struct, time, md5

hostname = '127.0.0.1'
port = 4950
#msg = "5a0006c02db09b6cf269d50c7f39bb23^what^120209^0^<sproutCast><title>dump on your face</title></sproutCast>"
host = socket.gethostbyname(hostname)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	


m = md5.new()
tempMessage = "what^120209^" + str(18) + "^<sproutCast><title>dump on your face</title></sproutCast>"
m.update(tempMessage);
msg = str(m.hexdigest()) + "^" + tempMessage
print msg
time.sleep(.01)
s.sendto(msg,(host,port))



m = md5.new()
tempMessage = "what^120209^" + str(23) + "^<sproutCast><title>dump on your face</title></sproutCast>"
m.update(tempMessage);
msg = str(m.hexdigest()) + "^" + tempMessage
print msg
time.sleep(.01)
s.sendto(msg,(host,port))
#test

m = md5.new()
tempMessage = "what^120209^" + str(27) + "^<sproutCast><title>dump on your face</title></sproutCast>"
m.update(tempMessage);
msg = str(m.hexdigest()) + "^" + tempMessage
print msg
time.sleep(.01)
s.sendto(msg,(host,port))