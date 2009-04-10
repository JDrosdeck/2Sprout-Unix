require 'sprout'
def getData()
  Sprout.getFeed();
end

def getNewEntry()
  s = Sprout.getNextItem()
  print s
end


t1 = Thread.new{getData()}
t2 = Thread.new{getNewEntry()}
t1.join
t2.join
