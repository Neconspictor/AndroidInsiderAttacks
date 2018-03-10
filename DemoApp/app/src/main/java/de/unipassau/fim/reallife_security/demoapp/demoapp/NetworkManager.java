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
     * Inits the SSL factory with the TLS algorithm.
     * The trust store is loaded from the provided key store file.
     * @param keyStoreFile The key store
     * @param pass The password for the key store file.
     * @throws KeyStoreException If an error occurs that is related with loading or initializing
     *  the key store.
     * @throws IOException If an IO error occurs
     * @throws CertificateException if any of the certificates in the keystore could not be loaded
     * @throws UnrecoverableKeyException If keys in the key store cannot be recovered
     *          (e.g. the given password is wrong).
     * @throws KeyManagementException if the ssl context cannot be initialized
     */
    private void initTLS(String keyStoreFile, String pass) throws
            KeyStoreException, IOException, CertificateException, UnrecoverableKeyException,
            KeyManagementException {
        InputStream keyStoreStream = readFromRawFolder(keyStoreFile);
        InputStream trustStoreStream = readFromRawFolder(keyStoreFile);

        KeyManagerFactory kmf = createAndInitKeyManagerFactory(keyStoreStream, pass);
        TrustManagerFactory tmf = createAndInitTrustManagerFactory(trustStoreStream, pass);

        sslContext = getTLSContext();
        sslContext.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
        SSLContext.setDefault(sslContext);

        factory = sslContext.getSocketFactory();
    }

    /**
     * Creates and initializes a TrustManagerFactory with a given key store (provided as input stream).
     * @param in The key store
     * @param password The password of the keystore
     *
     * @return A TrustManagerFactory that is initialized with the key store.
     *
     * @throws KeyStoreException If the created TrustManagerFactory could not be initialized.
     * @throws CertificateException if any of the certificates in the keystore could not be loaded
     * @throws IOException If any IO error occurs.
     */
    private TrustManagerFactory createAndInitTrustManagerFactory(InputStream in, String password) throws
            KeyStoreException,
            CertificateException,
            IOException {

        TrustManagerFactory tmf = getTrustManagerFactory();
        KeyStore trustStore = getPKCS12KeyStore();

        try {
            trustStore.load(in, password.toCharArray());
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("NoSuchAlgorithmException thrown for default algorithm!", e);
        }

        tmf.init(trustStore);
        return tmf;
    }


    /**
     * Creates and initializes a KeyManagerFactory with a given key store (provided as input stream).
     * @param in The key store
     * @param password The password of the keystore
     *
     * @return A KeyManagerFactory that is initialized with the key store.
     *
     * @throws IOException If any IO error occurs.
     * @throws CertificateException If any of the certificates in the keystore could not be loaded
     * @throws UnrecoverableKeyException If keys in the key store cannot be recovered
     *          (e.g. the given password is wrong).
     * @throws KeyStoreException If the created KeyManagerFactory could not be initialized.
     */
    private KeyManagerFactory createAndInitKeyManagerFactory(InputStream in, String password) throws
            IOException,
            CertificateException,
            UnrecoverableKeyException,
            KeyStoreException {
        KeyManagerFactory kmf;

        try {
            kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
            KeyStore keyStore = getPKCS12KeyStore();
            keyStore.load(in, password.toCharArray());
            kmf.init(keyStore, password.toCharArray());
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("NoSuchAlgorithmException thrown for default algorithm!", e);
        }

        return kmf;
    }


    /**
     * @param fileNameWithoutExtension The file name without file extension
     * @return An input stream from a file in the app's raw folder.
     */
    private InputStream readFromRawFolder(String fileNameWithoutExtension) {
        return resources.openRawResource(
                resources.getIdentifier(fileNameWithoutExtension,
                        "raw", packageName));
    }


    /**
     * @return A PKCS12 key store instance.
     */
    private KeyStore getPKCS12KeyStore() {
        try {
            return KeyStore.getInstance("PKCS12");
        } catch (KeyStoreException e) {
            throw new RuntimeException("PKCS12 KeyStore not found!", e);
        }
    }

    /**
     * @return A SSLContext that uses the TLS algorithm.
     */
    private SSLContext getTLSContext() {
        try {
            return SSLContext.getInstance("TLS");
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("TLS context not found!", e);
        }
    }

    /**
     * @return A TrustManagerFactory using the default algorithm.
     */
    private TrustManagerFactory getTrustManagerFactory() {
        try {
            return TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Couldn't get TrustManagerFactory instance for default algorithm!", e);
        }
    }
}