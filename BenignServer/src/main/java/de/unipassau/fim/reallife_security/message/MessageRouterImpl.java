package de.unipassau.fim.reallife_security.message;

import java.util.ArrayList;
import java.util.List;

/**
 * Default implementation of a message router.
 */
public class MessageRouterImpl implements MessageRouter {

  protected List<MessageListener> listeners = new ArrayList<>();

  public void addMessageListener(MessageListener listener) {
    synchronized (this) {
      listeners.add(listener);
    }
  }

  @Override
  public void route(Message message) {
    synchronized (this) {
     for (MessageListener listener : listeners) {
       listener.onMessageReceive(message);
     }
    }
  }

  @Override
  public void removeMessageListener(MessageListener listener) {
    synchronized (this) {
      listeners.remove(listener);
    }
  }
}