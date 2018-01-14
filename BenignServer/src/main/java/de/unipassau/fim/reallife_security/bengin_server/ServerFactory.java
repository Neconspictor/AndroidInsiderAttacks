package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.exception.InitException;
import de.unipassau.fim.reallife_security.message.MessageRouter;

/**
 * Created by David Goeth on 11.01.2018.
 */
public interface ServerFactory {

  Server create(MessageRouter router) throws InitException;
}