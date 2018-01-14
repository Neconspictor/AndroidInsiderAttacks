package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.FactoryException;
import de.unipassau.fim.reallife_security.exception.InitException;

import javax.net.ServerSocketFactory;
import javax.net.ssl.*;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.*;
import java.security.cert.CertificateException;

/**
 * Created by David Goeth on 11.01.2018.
 */
public abstract class AbstractTLSServer extends AbstractServer {

  protected String keyStoreFileName;
  protected String password;

  public AbstractTLSServer() {
    super();
  }

  @Override
  public void init() throws InitException {

    if (keyStoreFileName == null) throw new InitException("private keystore filename wasn't specified!");
    if (password == null) throw new InitException("password wasn't specified!");

    super.init();
  }

  public void setKeyStoreFileName(String keyStoreFileName) {
    this.keyStoreFileName = keyStoreFileName;
  }

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
    } catch (CertificateException | IOException | KeyManagementException | KeyStoreException
        | NoSuchAlgorithmException | UnrecoverableKeyException e) {
      throw new FactoryException(e);
    }
  }

  protected ServerSocketFactory createSSLFactory(InputStream keyStoreInputStream, String password) throws
      KeyStoreException, CertificateException, NoSuchAlgorithmException, IOException, UnrecoverableKeyException,
      KeyManagementException {

    //InputStream trustStoreInputStream = ClassLoader.class.getResourceAsStream("/publicKeystore.p12");

    KeyStore keyStore = KeyStore.getInstance("PKCS12");
    //keyStore.load(null, null);
    keyStore.load(keyStoreInputStream, password.toCharArray());

    KeyManagerFactory kmf =
        KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
    kmf.init(keyStore, password.toCharArray());

    //KeyStore trustStore = KeyStore.getInstance("PKCS12");
    //trustStore.load(trustStoreInputStream, "password".toCharArray());

    // init the trust networkManager factory by read certificates
    //TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
    //tmf.init(trustStore);

    SSLContext sslContext = SSLContext.getInstance("TLS");
    //sslContext.init(kmf.getKeyManagers(), null, null);
    //sslContext.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
    sslContext.init(kmf.getKeyManagers(), null, null);
    SSLContext.setDefault(sslContext);

    return sslContext.getServerSocketFactory();
  }
}