package de.unipassau.fim.reallife_security.demoapp.demoapp;

import android.content.res.Resources;
import android.os.AsyncTask;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.Closeable;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ConnectException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.List;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

/**
 * Created by necon on 19.12.2017.
 */

public class NetworkManager {

    private List<OnMessageReceiveListener> listeners = new ArrayList<>();
    private Resources resources;
    private String packageName;
    private SSLContext sslContext;
    private SSLSocketFactory factory;

    private static final String host = "10.0.2.2";
    private static final int port = 8080;

    public NetworkManager(Resources resources, String packageName) throws CertificateException,
            UnrecoverableKeyException, NoSuchAlgorithmException, KeyStoreException,
            KeyManagementException, IOException {
        this.resources = resources;
        this.packageName = packageName;
        initSSL("public_keystore", "password");
    }

    public void sendMessage(String msg) {
        new SendTask().execute(msg);
    }

    public synchronized void addOnMessageReceiveListener(OnMessageReceiveListener listener) {
        listeners.add(listener);
    }

    public interface OnMessageReceiveListener {

        void onReceive(String msg);
    }

    //pass a p12 or pfx file (file may be on classpath also)
    private void initSSL(String keyStoreFile, String pass) throws NoSuchAlgorithmException,
            KeyStoreException, IOException, CertificateException, UnrecoverableKeyException,
            KeyManagementException {
        InputStream keyStoreStream = readFromRawFolder(keyStoreFile);
        InputStream trustStoreStream = readFromRawFolder(keyStoreFile);


        KeyStore keyStore = KeyStore.getInstance("PKCS12");
        keyStore.load(null, null);
        keyStore.load(keyStoreStream, pass.toCharArray());

        KeyManagerFactory kmf =
                KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        kmf.init(keyStore, pass.toCharArray());



        KeyStore trustStore = KeyStore.getInstance("PKCS12");
        trustStore.load(trustStoreStream, pass.toCharArray());

        // init the trust networkManager factory by read certificates
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init(trustStore);

        // 3. init the SSLContext using kmf and tmf above
        sslContext = SSLContext.getInstance("TLS");
        //sslContext.init(kmf.getKeyManagers(), null, null);
        sslContext.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
        SSLContext.setDefault(sslContext);

        factory = sslContext.getSocketFactory();
    }

    private InputStream readFromRawFolder(String fileNameWithoutExtension) {
        return resources.openRawResource(
                resources.getIdentifier(fileNameWithoutExtension,
                        "raw", packageName));
    }


    private class SendTask extends AsyncTask<String, Void, String> {

        private Exception e;

        protected String doInBackground(String... messages) {
                String message = messages[0];
            //create a connection to the server and send the data
            String response = null;
            BufferedReader reader = null;
            BufferedWriter writer = null;
            try (Socket socket = factory.createSocket()) {

                socket.connect(new InetSocketAddress(host, port), 3000);

                writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
                writeMessage(message, writer);

                reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                response = readResponse(reader);

                writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
                writer.write("ok");
                writer.flush();

            } catch (IOException e) {
                this.e = e;
            } finally {
                closeSilently(reader);
                closeSilently(writer);
            }

            // Void cannot instantiated, so we must return null
            return response;
        }

        private String readResponse(BufferedReader reader) throws IOException {
            String response = null;

            String contentLengthLine = reader.readLine();
            int contentLength;
            try {
                contentLength = Integer.parseInt(contentLengthLine);
            } catch (NumberFormatException e) {
                throw new IOException("Couldn't retrieve content-length");
            }

            char[] buffer = new char[128];
            StringBuilder builder = new StringBuilder();

            while(contentLength > buffer.length) {
                int read = reader.read(buffer, 0, buffer.length);
                if (read != buffer.length) {
                    throw new IOException("Couldn't retrieve whole response!");
                }

                builder.append(buffer, 0, read);

                contentLength -= buffer.length;
            }

            //finally one last buffer read
            int read = reader.read(buffer, 0, contentLength);
            if (read != contentLength) {
                throw new IOException("Couldn't retrieve whole response!");
            }

            builder.append(buffer, 0, read);
            response = builder.toString();

            return response;
        }

        private void writeMessage(String message, BufferedWriter writer) throws IOException {
            int contentLength = message.length();
            writer.write(String.valueOf(contentLength));
            writer.newLine();
            writer.write(message);
            writer.flush();
        }

        protected void onPostExecute(String returnValue) {
            // TODO: check this.e
            // TODO: do something with the feed
            if (e != null && e instanceof ConnectException) {
                Log.e("SenTask", "Couldn't connect to server");
            }

            for (OnMessageReceiveListener listener : listeners) {
                listener.onReceive(returnValue);
            }
        }
    }

    private void closeSilently(Closeable closeable) {
        if (closeable == null) return;
        try {
            closeable.close();
        } catch (IOException e) {
        }
    }
}
