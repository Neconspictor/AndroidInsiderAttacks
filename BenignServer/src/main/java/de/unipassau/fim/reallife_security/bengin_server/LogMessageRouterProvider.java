package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.message.MessageRouter;
import de.unipassau.fim.reallife_security.message.MessageRouterImpl;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class LogMessageRouterProvider {

  private static MessageRouter instance;

  public static MessageRouter getLogMessageRouter() {
    if (instance == null) instance = new MessageRouterImpl();
    return instance;
  }
}