package de.unipassau.fim.reallife_security.message;

/**
 * Created by David Goeth on 14.01.2018.
 */
public class CommunicationMessage extends StringMessage {

  protected String description;

  public CommunicationMessage(String description, String message) {
    super(message);
    this.description = description;
  }

  public String getDescription() {
    return description;
  }
}