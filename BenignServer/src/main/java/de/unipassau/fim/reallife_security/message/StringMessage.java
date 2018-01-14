package de.unipassau.fim.reallife_security.message;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class StringMessage implements Message {

  protected String message;

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