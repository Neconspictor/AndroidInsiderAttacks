package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.InitException;
import de.unipassau.fim.reallife_security.message.MessageRouter;
import org.apache.log4j.Logger;

import java.io.Closeable;
import java.io.IOException;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class Model  {

  private static Logger logger = Logger.getLogger(Model.class);

  private MessageRouter router;

  private Server server;

  private ServerFactory factory;

  private SaveData data;

  public Model(MessageRouter router, SaveData data) {
    this.router = router;
    this.factory = null;
    this.data = data;
    server = null;
  }

  public ServerFactory getFactory() {
    return factory;
  }

  public MessageRouter getRouter() {
    return router;
  }

  public void setFactory(ServerFactory factory) {
    this.factory = factory;
  }

  public void setRouter(MessageRouter router) {
    this.router = router;
  }

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

  public void stopServer() {
    if (server != null && server.isRunning())
      try {
        server.shutdown(true);
      } catch (IOException e) {
        logger.error(e);
      }
  }

  public SaveData getData() {
    return new SaveData(data);
  }
}