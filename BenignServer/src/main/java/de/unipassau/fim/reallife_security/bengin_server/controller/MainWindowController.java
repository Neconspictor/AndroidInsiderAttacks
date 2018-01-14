package de.unipassau.fim.reallife_security.bengin_server.controller;


import de.unipassau.fim.reallife_security.bengin_server.Model;
import de.unipassau.fim.reallife_security.message.CommunicationMessage;
import de.unipassau.fim.reallife_security.message.LogMessage;
import de.unipassau.fim.reallife_security.message.Message;
import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.KeyValue;
import javafx.animation.Timeline;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;

import java.net.URL;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import javafx.scene.CacheHint;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.util.Duration;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.fxmisc.flowless.VirtualizedScrollPane;
import org.fxmisc.richtext.InlineCssTextArea;
import org.fxmisc.richtext.model.Paragraph;

public class MainWindowController implements Initializable {

  private static Logger logger = Logger.getLogger(MainWindowController.class);

  @FXML
  private InlineCssTextArea textView;

  @FXML
  private VirtualizedScrollPane<InlineCssTextArea> scrollPane;

  private Stage configureMenu;

  private Test messageContainer = new Test();

  private static int MAX_LINE_COUNT = 100000;


  @Override
  public void initialize(URL location, ResourceBundle resources) {
    logger.debug("initialize called!");
  }

  public void init(Stage configureMenu, Model model) {

    this.configureMenu = configureMenu;

    textView.setEditable(false);
    textView.setCache(true);
    textView.setCacheHint(CacheHint.SPEED);

    scrollPane.setCache(true);
    scrollPane.setCacheHint(CacheHint.SPEED);

      model.getRouter().addMessageListener((message -> {

            synchronized (messageContainer) {
              messageContainer.messageQueue.add(message);

              if (messageContainer.uiThreadIsReady) {
                messageContainer.uiThreadIsReady = false;
                Platform.runLater(() -> {
                  synchronized(messageContainer) {
                    addMessages();
                  }
                });
              }
            }

            while (messageContainer.messageQueue.size() > 200) {
              try {
                Thread.sleep(5);
              } catch (InterruptedException e) {
                e.printStackTrace();
              }
            }

          }));
    }

    @FXML
    public void handleConfigureOnAction(ActionEvent event) {
      configureMenu.show();
    }

  private void handleMessage(Message message) {

    if (message instanceof LogMessage) {
      handleLogMessage((LogMessage)message);
    } else if (message instanceof CommunicationMessage) {
      handleCommunicationMessage((CommunicationMessage)message);
    } else {
      handleMessageDefault(message);
    }

    int difference = textView.getCurrentParagraph() - MAX_LINE_COUNT - 1;
    if (difference > 0) {
      textView.deleteText(0,0, difference,0);
    }
  }

  private void handleCommunicationMessage(CommunicationMessage message) {
    String desc = message.getDescription();
    String content = message.getContent();

    LengthDiff diff = addText(textView, desc);
    textView.setStyle(diff.oldLength, diff.newLength, "-fx-fill: lightskyblue;");

    diff = addText(textView, content);
    textView.setStyle(diff.oldLength, diff.newLength, "-fx-fill: sandybrown;");
  }

  private void handleLogMessage(LogMessage message) {
    LengthDiff diff = addText(textView, message.getContent());
    Level level = message.getLevel();

    String style;
    if (level == Level.INFO) {
      style = "-fx-fill: rgb(243, 255, 196);";
    } else if (level == Level.ERROR) {
      style = "-fx-fill: rgb(252, 85, 85);";
    } else {
      style = "-fx-fill: white;";
    }

    textView.setStyle(diff.oldLength, diff.newLength, style);
  }

  private void handleMessageDefault(Message message) {
    LengthDiff diff = addText(textView, message.getContent());
    textView.setStyle(diff.oldLength, diff.newLength,"-fx-fill: white;");
  }

  private void addMessages() {
      boolean doAnimation = messageContainer.messageQueue.size() < 30;

      for (Message msg : messageContainer.messageQueue) {
        handleMessage(msg);
      }

      messageContainer.messageQueue.clear();
      messageContainer.uiThreadIsReady = true;

      Double valueEnd = scrollPane.totalHeightEstimateProperty().getValue();

      if (valueEnd == null) return;

      if (!doAnimation) {
        scrollPane.estimatedScrollYProperty().setValue(valueEnd);
        return;
      }
      Animation animation = new Timeline(
          new KeyFrame(Duration.millis(500),
              new KeyValue(scrollPane.estimatedScrollYProperty(), valueEnd)));
      animation.play();
  }

  private LengthDiff addText(InlineCssTextArea textView, String text) {
    LengthDiff diff =  new LengthDiff();
    diff.oldLength = textView.getLength();
    textView.appendText(text);
    diff.newLength = textView.getLength();
    return diff;
  }

  private static class Test{
    public List<Message> messageQueue = new LinkedList<>();

    public boolean uiThreadIsReady = true;
  }

  private static class LengthDiff {
    public int oldLength;
    public int newLength;
  }
}