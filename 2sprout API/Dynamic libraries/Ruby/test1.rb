require 'sprout'

def get2sproutData()
  Sprout.getFeed()
  i = 0
  while 1
  i = i + 1  
  s = Sprout.getSproutItem()
  print s
  print "i: " + i.to_s()
  end
end


t1 = Thread.new{get2sproutData()}

t1.join()