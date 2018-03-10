package de.unipassau.fim.reallife_security.message;


import org.apache.log4j.Level;

/**
 * A message that is intended to be logged.
 */
public class LogMessage implements Message {

  protected Level level;

  protected String content;

  /**
   * Creates a new log message.
   * @param message The message
   * @param level The log level of the message.
   */
  public LogMessage(String message, Level level) {
    this.content = message;
    this.level = level;
  }

  /**
   * @return The log level of the message.
   */
  public Level getLevel() {
    return level;
  }

  @Override
  public String getContent() {
    return content;
  }
}