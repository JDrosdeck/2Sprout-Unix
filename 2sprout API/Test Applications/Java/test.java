public class test implements Runnable
{
	public void run()
	{
		sprout.getFeed();
		int x = 0;
		for(;;)
		{
			x = x + 1;
			String xmlString = sprout.getSproutItem();
			System.out.println(xmlString);
			System.out.println(x);
		}
		
	}
	
	public static void main(String args[])
	{
		System.load("/Users/jonathandrosdeck/Desktop/2sprout/2sprout API/Dynamic libraries/Java/libsprout.so");
		
		(new Thread(new test())).start();
		
		
		
		
		
	}
	
	
	
	
}