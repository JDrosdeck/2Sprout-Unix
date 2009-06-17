public class test implements Runnable
{
	public void run()
	{

		for(;;)
		{
			String xmlString = sprout.getSproutItem();
			System.out.println(xmlString);
		}
		
	}
	
	public static void main(String args[])
	{
		System.load("/Users/jonathandrosdeck/Desktop/2sprout/2sprout API/Dynamic libraries/Java/libsprout.so");
		
		(new Thread(new test())).start();
		
		
		
		
		
	}
	
	
	
	
}