package de.unipassau.fim.reallife_security.exception;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class FactoryException extends Exception {

  public FactoryException(Throwable cause) {
    super(cause);
  }

  public FactoryException(String s) {
    super(s);
  }

  public FactoryException(String s, Throwable cause) {
    super(s, cause);
  }
}