package de.unipassau.fim.reallife_security.demoapp.demoapp;

/**
 * Created by David Goeth on 3/9/2018.
 */
public class SendTask {

  public SendTask(){}

  protected native String doInBackgroundBackup(String... messages);

  protected native void onPostExecuteBackup(String response);
}