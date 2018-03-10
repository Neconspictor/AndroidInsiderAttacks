package de.unipassau.fim.reallife_security.bengin_server;

import de.unipassau.fim.reallife_security.bengin_server.controller.ConfigureServerMenuController;
import de.unipassau.fim.reallife_security.bengin_server.controller.MainWindowController;
import de.unipassau.fim.reallife_security.message.MessageRouter;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

public class Main extends Application {

  private static Logger logger = Logger.getLogger(Application.class);

  private static MessageRouter router;

  static {
    router = MessageRouterProvider.getMessageRouter();
  }

  private Stage mainWindow;
  private Stage configureMenu;
  private ConfigureServerMenuController startServerController;

  /**
   * Starts the benign server.
   * @param args Not used
   */
  public static void main(String[] args) {
    launch(args);
  }

  @Override
  public void start(Stage primaryStage) throws Exception {

    SaveData saveData = new SaveData("save.json");
    try {
      saveData.read();
    }catch (FileNotFoundException e) {
      logger.debug("No savefile found");
    }

    Model model = new Model(router, saveData);
    configureMenu = createConfigureMenu(model);
    mainWindow = setupMainWindow(primaryStage, "Benign Server", configureMenu, model);

    mainWindow.setOnCloseRequest(e -> {
      model.stopServer();
      startServerController.onClose(e);
    });

    mainWindow.show();

    //model.getRouter().route(new StringMessage("test"));
  }

  private Stage createConfigureMenu(Model model) throws IOException {

    FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("/StartServerMenu.fxml"));
    Parent root = fxmlLoader.load();

    Stage stage = new Stage();
    stage.setScene(new Scene(root));
    stage.setResizable(false);
    //stage.setOnCloseRequest(e->e.consume());
    stage.setTitle("Configuring And Starting Server");
    stage.initModality(Modality.APPLICATION_MODAL);

    startServerController = fxmlLoader.getController();
    startServerController.init(stage, model);
    File file;

    return stage;
  }

  public Stage setupMainWindow(Stage stage, String title, Stage configureMenu, Model model) throws IOException {

    FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("/mainWindow.fxml"));
    Parent root = fxmlLoader.load();
    MainWindowController controller = fxmlLoader.getController();

    controller.init(configureMenu, model);

    stage.setTitle(title);

    stage.setScene(new Scene(root, 800, 600));

    return stage;
  }
}