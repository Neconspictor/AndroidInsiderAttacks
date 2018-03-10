package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.FactoryException;
import de.unipassau.fim.reallife_security.exception.InitException;
import org.apache.log4j.Logger;

import javax.net.ServerSocketFactory;
import javax.net.ssl.*;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.*;
import java.security.cert.CertificateException;

/**
 * A base server implementation for servers that use TLS encryption for the communication with the clients.
 */
public abstract class AbstractTLSServer extends AbstractServer {

  private static Logger logger = Logger.getLogger(AbstractTLSServer.class);

  protected String keyStoreFileName;
  protected String password;

  /**
   * Default constructor.
   */
  public AbstractTLSServer() {
    super();
  }

  @Override
  public void init() throws InitException {

    if (keyStoreFileName == null) throw new InitException("private keystore filename wasn't specified!");
    if (password == null) throw new InitException("password wasn't specified!");

    super.init();
  }

  /**
   * Sets the key store that should be used for creating TLS connections.
   * IMPORTANT: Changes effect only if the server is not initialized, yet.
   * @param keyStoreFileName The key store file name.
   */
  public void setKeyStoreFileName(String keyStoreFileName) {
    this.keyStoreFileName = keyStoreFileName;
  }

  /**
   * Sets the password of the key store file.
   * @param password The password of the key store file.
   */
  public void setPassword(String password) {
    this.password = password;
  }

  @Override
  protected ServerSocketFactory createFactory() throws FactoryException {
    InputStream keyStoreInputStream;

    try {
      keyStoreInputStream = new FileInputStream(keyStoreFileName);
    } catch (FileNotFoundException e) {
      throw new FactoryException(e);
    }

    try {
      return createSSLFactory(keyStoreInputStream, password);
    } catch (CertificateException
        | IOException
        | KeyManagementException
        | KeyStoreException
        | UnrecoverableKeyException e) {
      throw new FactoryException(e);
    }
  }

  /**
   * Creates a ServerSocketFactory that uses TLS encryption.
   * @param keyStoreInputStream The key store.
   * @param password The password for the key store.
   * @return A ServerSocketFactory that uses TLS encryption.
   * @throws KeyStoreException If an error occurs that is related with loading or initializing
   *  the key store.
   * @throws CertificateException If any of the certificates in the keystore could not be loaded
   * @throws IOException If an IO error occurs
   * @throws UnrecoverableKeyException If keys in the key store cannot be recovered
   *          (e.g. the given password is wrong).
   * @throws KeyManagementException If the ssl context cannot be initialized
   */
  protected ServerSocketFactory createSSLFactory(InputStream keyStoreInputStream, String password) throws
      KeyStoreException, CertificateException, IOException, UnrecoverableKeyException,
      KeyManagementException {

    KeyManagerFactory kmf = createAndInitKeyManagerFactory(keyStoreInputStream, password);

    SSLContext sslContext = getTLSContext();
    sslContext.init(kmf.getKeyManagers(), null, null);

    SSLContext.setDefault(sslContext);

    return sslContext.getServerSocketFactory();
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
}