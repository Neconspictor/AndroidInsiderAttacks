package de.unipassau.fim.reallife_security.message;

/**
 * A message that is exchanged between communication partners.
 */
public class CommunicationMessage extends StringMessage {

  protected String description;

  /**
   * Creates a new communication message.
   * @param description A description of the message.
   * @param message The message.
   */
  public CommunicationMessage(String description, String message) {
    super(message);
    this.description = description;
  }

  /**
   * @return The description of the message.
   */
  public String getDescription() {
    return description;
  }
}