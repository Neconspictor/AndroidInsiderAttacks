package de.unipassau.fim.reallife_security.demoapp.demoapp;

/**
 * An interface for observers which listens to received messages.
 */
public interface OnMessageReceiveListener {

    /**
     * Should be called if the observed object has received a string message.
     * @param msg The message that has been received.
     */
    void onReceive(String msg);
}