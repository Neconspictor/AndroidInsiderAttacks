package de.unipassau.fim.reallife_security.bengin_server;

import java.io.IOException;

/**
 * An interface that defines a simple contract a server should implement.
 */
public interface Server {

  /**
   * @return True is the server is running.
   */
  boolean isRunning();

  /**
   * Shuts down the server.
   * @param block specifies whether this method should be processed synchronous. If block is false, this method is
   *              processed asynchronously.
   * @throws IOException If an unrecoverable IO error occurs.
   */
  void shutdown(boolean block) throws IOException;

  /**
   * Starts the server.
   * @throws IOException If an unrecoverable IO error occurs.
   */
  void start() throws IOException;
}