package de.unipassau.fim.reallife_security.message;


import org.apache.log4j.Level;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class LogMessage implements Message {

  protected Level level;

  protected String content;

  public LogMessage(String message, Level level) {
    this.content = message;
    this.level = level;
  }

  public Level getLevel() {
    return level;
  }

  @Override
  public String getContent() {
    return content;
  }
}