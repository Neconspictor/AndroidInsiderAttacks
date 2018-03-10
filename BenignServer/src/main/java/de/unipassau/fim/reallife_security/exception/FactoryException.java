package de.unipassau.fim.reallife_security.exception;

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