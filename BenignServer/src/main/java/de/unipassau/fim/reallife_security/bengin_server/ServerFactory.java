package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.InitException;
import de.unipassau.fim.reallife_security.message.MessageRouter;

/**
 * A contract for creating server instances.
 */
public interface ServerFactory {

  /**
   * Creates a new Server and initializes it with a given messag router.
   * @param router The message router the server should use.
   * @return A server that uses the given message router.
   * @throws InitException If the server couldn't be initialized.
   */
  Server create(MessageRouter router) throws InitException;
}