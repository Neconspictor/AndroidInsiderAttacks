package de.unipassau.fim.reallife_security.exception;

/**
 * Created by David Goeth on 11.01.2018.
 */
public class InitException extends Exception {

  public InitException(String s) {
    super(s);
  }

  public InitException(String s, Exception e) {
    super(s, e);
  }
}