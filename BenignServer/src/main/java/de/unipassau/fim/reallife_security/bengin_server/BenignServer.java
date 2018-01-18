package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.InitException;
import de.unipassau.fim.reallife_security.message.CommunicationMessage;
import de.unipassau.fim.reallife_security.message.MessageRouter;
import de.unipassau.fim.reallife_security.message.StringMessage;
import org.apache.log4j.Logger;

import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocket;
import java.io.*;
import java.net.Socket;

/**
 * Created by David Goeth on 19.12.2017.
 */
public class BenignServer extends AbstractTLSServer {

  private static Logger logger = Logger.getLogger(BenignServer.class);

  private MessageRouter router;

  public BenignServer(MessageRouter router) {
    this.router = router;
  }

  @Override
  public void init() throws InitException {
    if (router == null) throw new InitException("router wasn't specified!");
    super.init();
  }

  @Override
  public void handleSocket(Socket socket) {

    if (socket instanceof SSLSocket) {

      logger.debug("SSL is used");
      SSLSocket ssl = (SSLSocket) socket;
      SSLSession session = ssl.getSession();
      logger.info("SSL Protocol: " + session.getProtocol());
      logger.info("SSL Cipher Suite: " + session.getCipherSuite());
    }

    if (socket == null) throw new NullPointerException("socket is null!");

      BufferedReader reader = null;
      BufferedWriter writer = null;

      try {
        InputStream inputStream = socket.getInputStream();

        reader = new BufferedReader(new InputStreamReader(inputStream));
        String message = readMessage(reader);
        router.route(new CommunicationMessage("Client says: ", message));

        writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
        message = "Client, server received your message: '" + message + "'";
        writeResponse(writer, message);
        router.route(new CommunicationMessage("Send response: ", message));

        //wait till the client closes the connection.
        reader = new BufferedReader(new InputStreamReader(inputStream));
        String response = reader.readLine();
        router.route(new CommunicationMessage("Client responses: ", response));

      } catch (Exception e) {
        logger.error(e);
      } finally {
        closeSilently(reader);
        closeSilently(writer);
        closeSilently(socket);
      }
  }

  private static void writeResponse(BufferedWriter writer, String message) throws IOException {
    writer.write(String.valueOf(message.length()));
    writer.newLine();
    writer.write(message);
    writer.flush();
  }

  private static String readMessage(BufferedReader reader) throws IOException {
    String response;

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
    if (closeable == null) return;
    try {
      closeable.close();
    } catch (IOException e) { }
  }
}