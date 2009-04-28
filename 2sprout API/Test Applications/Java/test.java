public class test implements Runnable
{
	public void run()
	{
		System.out.println("HERE");
		sprout.getFeed();
		String xmlString = sprout.getNextItem();
		System.out.println("WHAT");
		System.out.println(xmlString);
		
	}
	
	public static void main(String args[])
	{
		System.load("/Users/jonathandrosdeck/Desktop/2sprout/2sprout API/Dynamic libraries/Java/libsprout.so");
		
		(new Thread(new test())).start();
		
		
		
		
		
	}
	
	
	
	
}