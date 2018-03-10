package de.unipassau.fim.reallife_security.message;

/**
 * Contract for message listeners.
 */
public interface MessageListener {

  /**
   * Action that should be called if a new message arrives.
   * @param message The arrived message.
   */
  void onMessageReceive(Message message);
}
