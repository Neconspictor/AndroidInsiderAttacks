package de.unipassau.fim.reallife_security.logging;


import de.unipassau.fim.reallife_security.message.LogMessage;
import de.unipassau.fim.reallife_security.message.MessageRouter;
import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.Layout;
import org.apache.log4j.spi.LoggingEvent;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * An message appender for log4j so that log messages that are logged via log4j
 * Also can be accessed by message listeners. That can be used e.g. to log messages
 * in a gui text view.
 */
public class MessageAppender extends AppenderSkeleton {

  private MessageRouter router;

  protected String routerProvider;

  protected String method;

  /**
   * Default constructor.
   */
  public MessageAppender() {super();}


  public void activateOptions() {

    Class provider;
    try {
       provider  = Class.forName(routerProvider);
      Method targetMethod = provider.getMethod(method);
      router = (MessageRouter) targetMethod.invoke(null);
    } catch (ClassNotFoundException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
      throw new RuntimeException("Couldn't instantiate message router", e);
    }

    super.activateOptions();
  }

  @Override
  protected void append(LoggingEvent loggingEvent) {
    StringBuilder builder = new StringBuilder();
    String message = this.layout.format(loggingEvent);
    String[] stackTrace = loggingEvent.getThrowableStrRep();

    //if (stackTrace == null)
      //message = message.substring(0, message.length()-Layout.LINE_SEP.length());

    builder.append(message);

    if (super.layout.ignoresThrowable()) {
      if (stackTrace != null) {
        int len = stackTrace.length;

        for(int i = 0; i < len; ++i) {
          builder.append(stackTrace[i]);
          builder.append(Layout.LINE_SEP);
        }

        if (stackTrace.length > 0) {
          builder.delete(builder.length() - Layout.LINE_SEP_LEN, builder.length());
        }
      }
    }

    router.route(new LogMessage(builder.toString(), loggingEvent.getLevel()));
  }

  @Override
  public void close() {

  }

  @Override
  public boolean requiresLayout() {
    return true;
  }

  /**
   * @param routerProvider The name of class that provides a method for creating a MessageRouter.
   */
  public void setRouterProvider(String routerProvider) {
    this.routerProvider = routerProvider;
  }

  /**
   * @param method The name of a method for creating a MessageRouter.
   */
  public void setRouterProviderMethod(String method) {
    this.method = method;
  }

}