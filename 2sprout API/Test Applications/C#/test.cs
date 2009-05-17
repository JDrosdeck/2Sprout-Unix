
using System;
using System.Threading;

public class test{
	
	
	static void start2sprout()
	{
		sprout.getFeed();
		int x = 0;
		
		for(;;)
		{
			x = x+ 1;
			string data = sprout.getSproutItem();
			Console.WriteLine(data);
			Console.WriteLine(x);
		}
	}
	
	
	static void Main(string[] args)
	{	
		Thread thread1 = new Thread(new ThreadStart(start2sprout));
		thread1.Start();
	}


}