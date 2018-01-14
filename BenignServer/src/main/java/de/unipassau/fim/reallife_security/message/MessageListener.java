package de.unipassau.fim.reallife_security.message;

/**
 * Created by David Goeth on 10.01.2018.
 */
public interface MessageListener {

  void onMessageReceive(Message message);
}
