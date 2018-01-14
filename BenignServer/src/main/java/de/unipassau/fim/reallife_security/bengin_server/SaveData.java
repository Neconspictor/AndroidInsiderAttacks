package de.unipassau.fim.reallife_security.bengin_server;

import com.sun.xml.internal.ws.policy.privateutil.PolicyUtils;
import jdk.nashorn.internal.parser.JSONParser;

import javax.json.*;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by David Goeth on 12.01.2018.
 */
public class SaveData {
  private String privateKeyStoreFile = "";
  private String password = "";
  private int port = -1;
  private String fileName;

  public SaveData(String fileName) {
    this.fileName = fileName;
  }

  public SaveData(SaveData other) {
    fileName = other.fileName;
    privateKeyStoreFile = other.privateKeyStoreFile;
    password = other.password;
    port = other.port;
  }

  public void read() throws FileNotFoundException {
    FileInputStream save = new FileInputStream(fileName);
    JsonReader reader = Json.createReader(save);
    JsonObject root = reader.readObject();
    SaveData result = new SaveData(fileName);

    privateKeyStoreFile = root.getString("privateKeyStoreFile");
    password = root.getString("password");
    port = root.getInt("port");
  }

  public void store() throws IOException {
    try (JsonWriter writer = Json.createWriter(new FileOutputStream(fileName))) {
      JsonObjectBuilder builder = Json.createObjectBuilder();
      builder.add("privateKeyStoreFile", privateKeyStoreFile);
      builder.add("password", password);
      builder.add("port", port);
      JsonObject object = builder.build();
      writer.writeObject(object);
    }
  }

  public String getPrivateKeyStoreFile() {
    return privateKeyStoreFile;
  }

  public void setPrivateKeyStoreFile(String privateKeyStoreFile) {
    this.privateKeyStoreFile = privateKeyStoreFile;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public int getPort() {
    return port;
  }

  public void setPort(int port) {
    this.port = port;
  }

  @Override
  public String toString() {
    return "(" + privateKeyStoreFile + ", '" + password + "', " + port + ")";
  }
}
