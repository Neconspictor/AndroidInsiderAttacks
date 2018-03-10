package de.unipassau.fim.reallife_security.exception;

public class InitException extends Exception {

  public InitException(String s) {
    super(s);
  }

  public InitException(String s, Exception e) {
    super(s, e);
  }
}