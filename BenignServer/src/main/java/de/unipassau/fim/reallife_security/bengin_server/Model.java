package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.InitException;
import de.unipassau.fim.reallife_security.message.MessageRouter;
import org.apache.log4j.Logger;

import java.io.IOException;

/**
 * The data model for the bengin server. Manages the server and routes messages received from the server.
 */
public class Model  {

  private static Logger logger = Logger.getLogger(Model.class);

  private MessageRouter router;

  private Server server;

  private ServerFactory factory;

  private SaveData data;

  /**
   * Creates a new Model.
   * @param router Used for routing messages.
   * @param data Data for starting the server.
   */
  public Model(MessageRouter router, SaveData data) {
    this.router = router;
    this.factory = null;
    this.data = data;
    server = null;
  }

  /**
   *
   * @return The used server factory.
   */
  public ServerFactory getFactory() {
    return factory;
  }

  /**
   *
   * @return The used message router.
   */
  public MessageRouter getRouter() {
    return router;
  }

  /**
   * Sets the factory for creating the server.
   * @param factory The server factory to use for the server.
   */
  public void setFactory(ServerFactory factory) {
    this.factory = factory;
  }

  /**
   * Sets the router to use for routing messages.
   * @param router The message router to use.
   */
  public void setRouter(MessageRouter router) {
    this.router = router;
  }

  /**
   * Action that starts the server.
   * @throws IOException If an IO error occurs.
   */
  public void startServer() throws IOException{

    if (factory == null) throw new IOException("No ServerFactory was set!");

    if (router == null) throw new IOException("No MessageRouter was set!");

    if (server != null && server.isRunning()) {
      throw new IOException("Server is still running!");
    }

    try {
      server = factory.create(router);
    } catch (InitException e) {
      throw new IOException(e);
    }
    server.start();
  }

  /**
   * Stops the server. Method blocks.
   */
  public void stopServer() {
    if (server != null && server.isRunning())
      try {
        server.shutdown(true);
      } catch (IOException e) {
        logger.error(e);
      }
  }

  /**
   * @return The used Save Data that are stored on disk.
   */
  public SaveData getData() {
    return new SaveData(data);
  }
}