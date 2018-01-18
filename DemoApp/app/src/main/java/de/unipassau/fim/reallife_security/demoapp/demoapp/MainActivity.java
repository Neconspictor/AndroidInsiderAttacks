package de.unipassau.fim.reallife_security.demoapp.demoapp;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.text.Editable;
import android.util.Log;
import android.util.TypedValue;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;



public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("evil-lib");
    }

    private NetworkManager networkManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);


        try {
            networkManager = new NetworkManager(getResources(), getPackageName());
        } catch (Exception e) {
            showFatalMessageBox("Couldn't init TLS", e);
            return;
        }

        final Button button = findViewById(R.id.sendButton);
        final EditText myEditText = ((EditText)findViewById(R.id.inputField));

        final Activity activity = this;

        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Editable text = myEditText.getText();
                if (text != null) {

                    String content = text.toString();
                    text.clear();

                    //Snackbar.make(v, "is send:" + content, Snackbar.LENGTH_LONG).setAction("Action", null).show();

                    //send message
                    networkManager.sendMessage(content);

                    LinearLayout linearLayout =  findViewById(R.id.log);
                    TextView textView = new TextView(activity);
                    textView.setText("Me: " + content);
                    textView.setTextAppearance(R.style.TextAppearance_AppCompat);
                    textView.setTextColor(Color.BLACK);
                    textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
                    linearLayout.addView(textView);

                    //scroll automatically to the end
                    final ScrollView scrollView = findViewById(R.id.log_scroll_view);
                    scrollView.post(new Runnable() {
                        @Override
                        public void run() {
                            scrollView.fullScroll(ScrollView.FOCUS_DOWN);
                        }
                    });
                }

                InputMethodManager inputManager = (InputMethodManager)
                        getSystemService(Context.INPUT_METHOD_SERVICE);

                inputManager.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(),
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        });

        networkManager.addOnMessageReceiveListener(new OnMessageReceiveListener() {
            @Override
            public void onReceive(String msg) {
                if (msg == null) {
                    Log.e("listener", "Couldn't send message");
                } else {
                    Log.d("listener", msg);
                }

                if (msg == null) {
                    msg = "SERVER ERROR";
                }


                LinearLayout linearLayout =  findViewById(R.id.log);
                TextView textView = new TextView(activity);
                textView.setText("Server: " + msg);
                textView.setTextAppearance(R.style.TextAppearance_AppCompat);
                textView.setTextColor(Color.MAGENTA);
                textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
                linearLayout.addView(textView);

                //scroll automatically to the end
                final ScrollView scrollView = findViewById(R.id.log_scroll_view);
                scrollView.post(new Runnable() {
                    @Override
                    public void run() {
                        scrollView.fullScroll(ScrollView.FOCUS_DOWN);
                    }
                });
            }
        });

        stringFromJNI();
        //Test.test();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_activateHooks) {
            activateHookJNI();
            return true;
        }

        if (id == R.id.action_deactivateHooks) {
            deactivateHookJNI();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public native String stringFromJNI();

    private native void activateHookJNI();

    private native void deactivateHookJNI();

    //*********************************************************
//generic dialog, takes in the method name and error message
//*********************************************************
    private void showFatalMessageBox(String method, Exception e)
    {
        final Exception copy = e;
        Log.wtf("MainActivity::onCreate", "Couldn't init TLS", copy);
        final Activity activity = this;
        AlertDialog.Builder messageBox = new AlertDialog.Builder(this);
        messageBox.setTitle(method);
        messageBox.setMessage(e.getMessage());
        messageBox.setCancelable(false);
        messageBox.setNeutralButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                //Log.e("", "Clicked!");
                //Log.e("MainActivity::onCreate", "Couldn't init TLS", copy);
                //activity.finishAndRemoveTask ();
                activity.finishAffinity();
            }
        });
        messageBox.show();
    }
}
