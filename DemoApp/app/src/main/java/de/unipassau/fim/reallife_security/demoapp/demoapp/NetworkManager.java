package de.unipassau.fim.reallife_security.demoapp.demoapp;

import android.content.res.Resources;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
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
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

/**
 * Handles the network layer of the benign app.
 */

public class NetworkManager {

    private List<OnMessageReceiveListener> listeners = new ArrayList<>();
    private Resources resources;
    private String packageName;
    private SSLContext sslContext;
    private SSLSocketFactory factory;

    public static final String host = "10.0.2.2";
    public static final int port = 8080;

    public NetworkManager(Resources resources, String packageName) throws CertificateException,
            UnrecoverableKeyException, NoSuchAlgorithmException, KeyStoreException,
            KeyManagementException, IOException {
        this.resources = resources;
        this.packageName = packageName;
        initSSL("public_keystore", "password");
    }

    /**
     * Sends a text message to the remote server.
     * Performs asynchonously.
     * @param msg The text message to be send.
     */
    public void sendMessage(String msg) {
        new SendTask(listeners,
                factory,
                new InetSocketAddress(NetworkManager.host, NetworkManager.port),
                3000
        ).execute(msg);
    }

    /**
     * Adds a listener for received server responses.
     * @param listener The listener that should be informed.
     */
    public synchronized void addOnMessageReceiveListener(OnMessageReceiveListener listener) {
        listeners.add(listener);
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
}
