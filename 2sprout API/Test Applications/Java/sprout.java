/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.31
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


public class sprout {
  public static int startFeed() {
    return sproutJNI.startFeed();
  }

  public static int stopFeed() {
    return sproutJNI.stopFeed();
  }

  public static String getNextItem() {
    return sproutJNI.getNextItem();
  }

  public static int getFeed() {
    return sproutJNI.getFeed();
  }

}
