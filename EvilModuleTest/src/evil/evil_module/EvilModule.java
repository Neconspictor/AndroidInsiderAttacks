package evil.evil_module;

import android.util.Log;

public class EvilModule {

    public static void foo() {
        Log.e("EvilModule", "foo() called!");
    }

    public native static void fooNative();
}