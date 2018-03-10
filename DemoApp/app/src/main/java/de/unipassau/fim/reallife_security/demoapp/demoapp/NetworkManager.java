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
 * The network layer of the benign demo app.
 */

public class NetworkManager {

    private List<OnMessageReceiveListener> listeners = new ArrayList<>();
    private Resources resources;
    private String packageName;
    private SSLContext sslContext;
    private SSLSocketFactory factory;

    /**
     * For the android emulator this is the hst name that means the underlying host machine.
     */
    public static final String host = "10.0.2.2";

    /**
     * The port number on that the remote server provides its service.
     */
    public static final int port = 8080;

    private static final String PUBLIC_KEYSTORE_FILE = "public_keystore";
    private static final String PUBLIC_KEYSTORE_PASSWORD = "password";

    /**
     * Creates a new NetworkManager object.
     * @param resources The app's resources
     * @param packageName The package name of the app
     *
     * @throws IOException If the network manager couldn't be initialized.
     */
    public NetworkManager(Resources resources, String packageName) throws IOException {
        this.resources = resources;
        this.packageName = packageName;
        try {
            initTLS(PUBLIC_KEYSTORE_FILE, PUBLIC_KEYSTORE_PASSWORD);
        } catch (CertificateException
                | NoSuchAlgorithmException
                | IOException
                | KeyManagementException
                | KeyStoreException
                | UnrecoverableKeyException e) {
            throw new IOException("Couldn't initialize TLS. Cause:", e);
        }
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
    

    /**
     *
     * @param keyStoreFile
     * @param pass
     * @throws NoSuchAlgorithmException
     * @throws KeyStoreException
     * @throws IOException
     * @throws CertificateException If an error occurs while negotiating SSL certificates
     * @throws UnrecoverableKeyException
     * @throws KeyManagementException
     */
    private void initTLS(String keyStoreFile, String pass) throws NoSuchAlgorithmException,
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