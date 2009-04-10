
using System;
using System.Threading;

public class test{
	
	
	static void startAPI()
	{
		sprout.getFeed();
	}
	
	static void getData()
	{
	string x = sprout.getNextItem();
	Console.WriteLine(x);
	
	}
	
	
	static void Main(string[] args)
	{	
		Thread thread1 = new Thread(new ThreadStart(startAPI));
		Thread thread2 = new Thread(new ThreadStart(getData));
		thread1.Start();
		thread2.Start();
	}


}