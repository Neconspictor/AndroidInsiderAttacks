package evil.evil_module;

import java.io.BufferedReader;
import java.io.IOException;

public class EvilModule {

    //protected String doInBackground(String... messages)
    protected native String doInBackgroundHook(String... messages);

    protected native String doInBackgroundBackup(String... messages);

    //private String readResponse(BufferedReader reader) throws IOException
    private native String readResponseHook(BufferedReader reader) throws IOException;

    private native String readResponseBackup(BufferedReader reader) throws IOException;
}