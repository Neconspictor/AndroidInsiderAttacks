package de.unipassau.fim.reallife_security.message;


/**
 * Contract for arbitrary messages.
 */
public interface Message {

  /**
   * @return Text represention of this message.
   */
  String getContent();
}