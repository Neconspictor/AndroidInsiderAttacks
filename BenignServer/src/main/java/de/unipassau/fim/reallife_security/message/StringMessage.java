package de.unipassau.fim.reallife_security.message;

/**
 * A message that contains text content.
 */
public class StringMessage implements Message {

  protected String message;

  /**
   * Creates a new string message.
   * @param message
   */
  public StringMessage(String message) {
    this.message = message + System.getProperty("line.separator");
  }

  @Override
  public String getContent() {
    return message;
  }

  @Override
  public String toString() {
    return message;
  }
}