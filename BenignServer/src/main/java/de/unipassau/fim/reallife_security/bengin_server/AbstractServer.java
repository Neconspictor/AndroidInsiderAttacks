package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.FactoryException;
import de.unipassau.fim.reallife_security.exception.InitException;
import org.apache.log4j.Logger;

import javax.net.ServerSocketFactory;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/**
 * Implements default behaviour for servers used in the 'Android Insider Attacks' project.
 */
public abstract class AbstractServer implements Server {

  private static Logger logger = Logger.getLogger(Server.class);

  protected ServerSocket serverSocket;

  protected ExecutorService executorService;

  protected boolean initialized = false;

  protected int threadPoolSize = -1;

  protected int port = -1;


  /**
   * Default constructor.
   */
  public AbstractServer()  { }

  /**
   * @return The port number this server is running on.
   */
  public int getPort() {
    return port;
  }

  /**
   * @return The size of the used thread pool for serving multiple clients i parallel.
   */
  public int getThreadPoolSize() {
    return threadPoolSize;
  }

  /**
   * Action that should be called if a client connects to the server.
   * @param socket The connection to the client.
   */
  public abstract void handleSocket(Socket socket);

  @Override
  public void start() throws IOException {

    if (!initialized) throw new IOException("AbstractServer is not initialized");

    logger.debug("Start server...");

    executorService.execute(()->serve());

    logger.info("BenignServer is running.");
  }

  @Override
  public void shutdown(boolean block) throws IOException {
    if (!initialized) throw new IOException("BenignServer not initialized!");

    logger.debug("Initiate shutdown...");

    try {
      serverSocket.close();
    } catch (IOException e) {
      logger.debug(e);
    }
    executorService.shutdown();
    initialized = false;
    if (block) {
      try {
        while(!executorService.awaitTermination(10, TimeUnit.DAYS)){}
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    logger.info("BenignServer is successfully shutdown.");
  }

  @Override
  public boolean isRunning() {
    if (serverSocket == null) return false;
    return !serverSocket.isClosed();
  }

  /**
   * Initializes this server. This method has to be called before the server is started.
   * @throws InitException If the server couldn not be initialized.
   */
  public void init() throws InitException {

    if (initialized) throw new InitException("AbstractServer is already initialized!");

    if (threadPoolSize <= 0) throw new InitException("thread pool size isn't set.");

    if (port <= 0) throw new InitException("port number isn't set.");

    threadPoolSize = Integer.max(threadPoolSize, 1);

    ServerSocketFactory serverSocketFactory = null;
    try {
      serverSocketFactory = createFactory();
      serverSocket =  serverSocketFactory.createServerSocket(port);
    } catch (FactoryException | IOException e) {
      throw new InitException("Couldn't create server socket", e);
    }

    //we need one additional thread for accepting clients
    executorService = Executors.newFixedThreadPool(threadPoolSize + 1);

    initialized = true;
  }

  /**
   * Creates a factory for creating a server socket, that is used by this server.
   * @return
   * @throws FactoryException
   */
  protected abstract ServerSocketFactory createFactory() throws FactoryException;

  /**
   * Main loop. Incoming client connections are accepted by the created server socket.
   * And than the connection is served by the method 'handleSocket'
   */
  protected void serve() {
    while (!serverSocket.isClosed()) {
      try {
        logger.debug("Waiting for client...");
        final Socket socket = serverSocket.accept();
        logger.debug("Accepted client.");
        executorService.execute(()->handleSocket(socket));
        logger.debug("served client.");
      } catch (IOException e) {
        if (isRunning())
          logger.error(e);
        else
          logger.debug("SSL BenignServer Socket was interrupted while listening for clients.");
      }
    }

    logger.info("Finished serving.");
  }

  /**
   * Sets the port where this server should run on.
   * IMPORTANT: Changes effect only if the server is not initialized, yet.
   *
   * @param port The port this server should run on
   */
  public void setPort(int port) {
    this.port = port;
  }

  /**
   * Sets the thread pool size that determines how much clients can be served in parallel.
   * IMPORTANT: Changes effect only if the server is not initialized, yet.
   *
   * @param threadPoolSize The thread pool size to use.
   */
  public void setThreadPoolSize(int threadPoolSize) {
    this.threadPoolSize = threadPoolSize;
  }
}