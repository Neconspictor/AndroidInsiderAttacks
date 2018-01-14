package de.unipassau.fim.reallife_security.message;

/**
 * Created by David Goeth on 11.01.2018.
 */
public interface MessageRouter {

  void addMessageListener(MessageListener listener);

  void route(Message message);

  void removeMessageListener(MessageListener listener);
}