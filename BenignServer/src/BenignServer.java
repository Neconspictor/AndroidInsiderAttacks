import javax.net.ssl.*;
import java.io.*;
import java.net.Socket;
import java.security.*;
import java.security.cert.CertificateException;

/**
 * Created by David Goeth on 19.12.2017.
 */
public class BenignServer {

  private static final String KEYSTORE_PASSWORD = "password";
  private static final String KEYSTORE_FILE_NAME = "/privateKeystore.p12";

  public static void main(String[] args)  {

    SSLServerSocket sslServerSocket = null;
    try {
      sslServerSocket = createServerSocket();
    } catch (IOException e) {
      System.err.println("Error, while creating server socket: ");
      e.printStackTrace();
      System.out.println("Aborting...");
      return;
    }

    while (true) {
      try {
        SSLSocket sslSocket = (SSLSocket) sslServerSocket.accept();
        handleSocket(sslSocket);
      } catch (IOException e) {

      }
    }
  }

  private static SSLServerSocket createServerSocket() throws IOException {
    SSLServerSocketFactory sslServerSocketFactory = null;
    try {
      sslServerSocketFactory = createFactory(KEYSTORE_FILE_NAME, KEYSTORE_PASSWORD.toCharArray());
      return (SSLServerSocket) sslServerSocketFactory.createServerSocket(8080);
    } catch (KeyStoreException | CertificateException | NoSuchAlgorithmException | IOException |
        UnrecoverableKeyException | KeyManagementException e) {
      throw new IOException("Couldn't create server socket!", e);
    }
  }

  private static void handleSocket(SSLSocket sslSocket) {
    new Thread(() -> {
      BufferedReader reader = null;
      BufferedWriter writer = null;

      try {
        InputStream inputStream = sslSocket.getInputStream();

        reader = new BufferedReader(new InputStreamReader(inputStream));
        String message = readMessage(reader);
        System.out.println("Client says: " + message);

        writer = new BufferedWriter(new OutputStreamWriter(sslSocket.getOutputStream()));
        writeResponse(writer, "received message: " + message);
        System.out.println("Send response successfully.");

        //wait till the client closes the connection.
        reader = new BufferedReader(new InputStreamReader(inputStream));
        String response = reader.readLine();
        System.out.println("Client responses: " + response);

      } catch (Exception e) {
        e.printStackTrace();
      } finally {
        closeSilently(reader);
        closeSilently(writer);
        closeSilently(sslSocket);
      }
    }).start();
  }

  private static void writeResponse(BufferedWriter writer, String message) throws IOException {
    writer.write(String.valueOf(message.length()));
    writer.newLine();
    writer.write(message);
    writer.flush();
  }

  private static String readMessage(BufferedReader reader) throws IOException {
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

  private static void closeSilently(Closeable closeable) {
    try {
      closeable.close();
    } catch (IOException e) { }
  }

  private static SSLServerSocketFactory createFactory(String keyStoreFileName, char[] password) throws KeyStoreException,
      CertificateException, NoSuchAlgorithmException, IOException, UnrecoverableKeyException, KeyManagementException {

    InputStream keyStoreInputStream = ClassLoader.class.getResourceAsStream(keyStoreFileName);

    //InputStream trustStoreInputStream = ClassLoader.class.getResourceAsStream("/publicKeystore.p12");

    KeyStore keyStore = KeyStore.getInstance("PKCS12");
    keyStore.load(null, null);
    keyStore.load(keyStoreInputStream, password);

    KeyManagerFactory kmf =
        KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
    kmf.init(keyStore, password);

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