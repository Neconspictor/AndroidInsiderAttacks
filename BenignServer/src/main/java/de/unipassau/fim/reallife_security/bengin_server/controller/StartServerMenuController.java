package de.unipassau.fim.reallife_security.bengin_server.controller;

import de.unipassau.fim.reallife_security.bengin_server.*;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;

/**
 * Created by David Goeth on 12.01.2018.
 */
public class StartServerMenuController implements Initializable {

  private static Logger logger = Logger.getLogger(StartServerMenuController.class);

  private static int UNSIGNED_SHORT_SIZE = (int) Math.pow(2,16);

  private Stage root;

  private SaveData data;

  private Model model;

  @FXML
  private TextField selectedPrivateKeystore;

  @FXML
  private PasswordField passwordField;

  @FXML
  private TextField portTextField;

  public void init(Stage root, Model model) {
    this.root = root;
    this.model = model;

    data = model.getData();
    if (data == null) return;

    selectedPrivateKeystore.setText(data.getPrivateKeyStoreFile());
    passwordField.setText(data.getPassword());
    if (data.getPort() >= 0)
      portTextField.setText(String.valueOf(data.getPort()));
  }

  public void onClose(WindowEvent event) {
    try {
      data.setPrivateKeyStoreFile(selectedPrivateKeystore.getText());
      data.setPassword(passwordField.getText());
      String portText = portTextField.getText();
      if (portText.equals("") ) portText = "-1";
      int port = Integer.parseInt(portText);
      data.setPort(port);
      data.store();
      logger.debug("Stored save data: " + data);
    } catch (IOException | NumberFormatException e) {
      logger.error("Couldn't store save data", e);
    }
  }

  @Override
  public void initialize(URL location, ResourceBundle resources) {

    portTextField.textProperty().addListener((observable, oldValue, newValue) -> {
      if(!newValue.matches("[0-9]{0,5}")){
        portTextField.setText(oldValue);
      } else if (!newValue.equals("")){
        int currentNumber= 0;
        try {
          currentNumber = Integer.parseInt(newValue);
        } catch (NumberFormatException e) {
          portTextField.setText(oldValue);
          logger.error(e);
          return;
        }

        if (currentNumber >= UNSIGNED_SHORT_SIZE) {
          portTextField.setText(oldValue);
        }
      }
    });
  }

  @FXML
  public void startServer(ActionEvent event) {

    File privateKeyStore = new File(selectedPrivateKeystore.getText());
    String password = passwordField.getText();
    String portText = portTextField.getText();
    if (portText.equals("")) portText = "0";
    final int port;
    try{
      port = Integer.parseInt(portText);
    } catch (NumberFormatException e) {
      logger.error("Couldn't parse port number.", e);
      logger.error("Aborting server start process...");
      return;
    }

    logger.debug("Chosen private key store: " + privateKeyStore);
    logger.debug("Private key store exists: " + privateKeyStore.exists());
    logger.debug("Password: " + password);
    logger.debug("Port: " + port);

    model.setFactory(router -> {
      BenignServer server =  new BenignServer(router);
      server.setKeyStoreFileName(privateKeyStore.getAbsolutePath());
      server.setPassword(password);
      server.setThreadPoolSize(5);
      server.setPort(port);
      server.init();

      return server;
    });

    try {
      model.stopServer();
      model.startServer();
    } catch (IOException e) {
      logger.error("Couldn't start server", e);
    }

    root.hide();
  }

  @FXML
  public void selectPrivateKeystore(ActionEvent event) {
    FileChooser fileChooser = new FileChooser();
    //FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("TXT files (*.txt)", "*.txt");
    //fileChooser.getExtensionFilters().add(extFilter);
    String path = selectedPrivateKeystore.getText();
    File folder = new File(path);
    path = folder.getParent();
    if (path == null || path.equals("")) path = "./";
    fileChooser.setInitialDirectory(new File(path));
    File file = fileChooser.showOpenDialog(root);

    if (file == null) return;

    Platform.runLater(()->{
      selectedPrivateKeystore.setText(file.getPath());
    });

    logger.debug("Chose private key store: " + file.getPath());
  }
}