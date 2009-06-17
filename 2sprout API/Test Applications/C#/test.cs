
using System;
using System.Threading;

public class test{
	
	
	static void start2sprout()
	{
		
		for(;;)
		{
			string data = sprout.getSproutItem();
			Console.WriteLine(data);
		}
	}
	
	
	static void Main(string[] args)
	{	
		Thread thread1 = new Thread(new ThreadStart(start2sprout));
		thread1.Start();
	}


}