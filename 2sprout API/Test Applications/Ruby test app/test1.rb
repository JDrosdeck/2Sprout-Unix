require 'sprout'

def get2sproutData()
  while 1
  s = Sprout.getSproutItem()
  print s
  print "\n"
  end
end


t1 = Thread.new{get2sproutData()}

t1.join()