package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.message.MessageRouter;
import de.unipassau.fim.reallife_security.message.MessageRouterImpl;

/**
 * A provider for a static message router.
 */
public class MessageRouterProvider {

  private static MessageRouter instance;

  /**
   * @return The message router of this class.
   */
  public static MessageRouter getMessageRouter() {
    if (instance == null) instance = new MessageRouterImpl();
    return instance;
  }
}