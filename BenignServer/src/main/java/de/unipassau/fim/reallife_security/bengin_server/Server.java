package de.unipassau.fim.reallife_security.bengin_server;

import java.io.IOException;

/**
 * Created by David Goeth on 11.01.2018.
 */
public interface Server {

  boolean isRunning();

  void shutdown(boolean block) throws IOException;

  void start() throws IOException;
}