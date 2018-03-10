package evil.evil_module;

/**
 * The evil module that is used as helper class for hooking.
 */
public class EvilModule {

    //protected String doInBackground(String... messages)
    public native String doInBackgroundHook(String... messages);
    public native String doInBackgroundBackup(String... messages);

    //protected void onPostExecute(String returnValue)
    protected native void onPostExecuteHook(String returnValue);
    protected native void onPostExecuteBackup(String returnValue);
}