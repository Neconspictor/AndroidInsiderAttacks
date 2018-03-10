package de.unipassau.fim.reallife_security.message;

/**
 * An interface for classes that want to send a received message to a set of listeners.
 */
public interface MessageRouter {

  /**
   * Adds a message listener.
   * @param listener The listener to add.
   */
  void addMessageListener(MessageListener listener);

  /**
   * Sends a message to all registered listeners.
   * @param message The message to route.
   */
  void route(Message message);

  /**
   * Removes a message listener.
   * @param listener The listener to remove.
   */
  void removeMessageListener(MessageListener listener);
}