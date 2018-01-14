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
 * Created by David Goeth on 11.01.2018.
 */
public abstract class AbstractServer implements Server {

  private static Logger logger = Logger.getLogger(Server.class);

  protected ServerSocket serverSocket;

  protected ExecutorService executorService;

  protected boolean initialized = false;

  protected int threadPoolSize = -1;

  protected int port = -1;


  public AbstractServer()  { }

  public int getPort() {
    return port;
  }

  public int getThreadPoolSize() {
    return threadPoolSize;
  }

  public abstract void handleSocket(Socket sslSocket);

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

  //protected abstract ServerSocketFactory createFactory(String keyStoreFileName, String password) throws FactoryException;

  /*protected synchronized void route(String message, Category category) {
    for (MessageListener listener : listeners) {
      listener.onMessageReceive(message, category);
    }
  }*/

  /*protected void route(Exception e, Category category) {
    MessageListener listener = (message, catagoryCallback) -> route(message, catagoryCallback);
    MessageListenerPrintStream printStream = new MessageListenerPrintStream(listener);
    e.printStackTrace(printStream);
    printStream.submit(category);
  }*/

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

  protected abstract ServerSocketFactory createFactory() throws FactoryException;

  protected void serve() {
/*
    long timeBefore = System.nanoTime();

    for (int i = 0; i < 10000; ++i) {
      logger.debug("Hello World: " + i);
    }

    long diff = System.nanoTime() - timeBefore;

    System.out.println("Needed time: " + diff / 1000000000.0d + " seconds");
*/
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

  public void setPort(int port) {
    this.port = port;
  }

  public void setThreadPoolSize(int threadPoolSize) {
    this.threadPoolSize = threadPoolSize;
  }
}