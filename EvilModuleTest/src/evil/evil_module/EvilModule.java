package evil.evil_module;

public class EvilModule {

    //protected String doInBackground(String... messages)
    public native String doInBackgroundHook(String... messages);

    public native String doInBackgroundBackup(String... messages);

    //protected void onPostExecute(String returnValue)
    protected native void onPostExecuteHook(String returnValue);
    protected native void onPostExecuteBackup(String returnValue);

    //private String readResponse(BufferedReader reader) throws IOException
    //private native String readResponseHook(BufferedReader reader) throws IOException;
    //private native String readResponseBackup(BufferedReader reader) throws IOException;
}