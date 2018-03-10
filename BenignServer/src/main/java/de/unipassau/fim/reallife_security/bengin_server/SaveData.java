package de.unipassau.fim.reallife_security.bengin_server;

import javax.json.*;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Container for storing data that are used to create a server instance.
 * This data can be stored and read from disk.
 */
public class SaveData {
  private String privateKeyStoreFile = "";
  private String password = "";
  private int port = -1;
  private String fileName;

  /**
   * Creates a new SaveData object.
   * @param fileName The file this object should use for storing and reading data.
   */
  public SaveData(String fileName) {
    this.fileName = fileName;
  }

  /**
   * Copy constructor.
   * @param other The other SaveData used to initialize this object.
   */
  public SaveData(SaveData other) {
    fileName = other.fileName;
    privateKeyStoreFile = other.privateKeyStoreFile;
    password = other.password;
    port = other.port;
  }

  /**
   * Reads the data from file.
   * @throws FileNotFoundException If the 'fileName' attribute this object is created with, is not a valid file.
   */
  public void read() throws FileNotFoundException {
    FileInputStream save = new FileInputStream(fileName);
    JsonReader reader = Json.createReader(save);
    JsonObject root = reader.readObject();

    privateKeyStoreFile = root.getString("privateKeyStoreFile");
    password = root.getString("password");
    port = root.getInt("port");
  }

  /**
   * Stores this object to disk using json serialization.
   * @throws IOException If an IO error occurs.
   */
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

  /**
   * @return The (private) key store file this object uses.
   */
  public String getPrivateKeyStoreFile() {
    return privateKeyStoreFile;
  }

  /**
   * @param privateKeyStoreFile The (private) key store file this object should use.
   */
  public void setPrivateKeyStoreFile(String privateKeyStoreFile) {
    this.privateKeyStoreFile = privateKeyStoreFile;
  }

  /**
   * @return The password this object uses.
   */
  public String getPassword() {
    return password;
  }

  /**
   * @param password The password this object should use.
   */
  public void setPassword(String password) {
    this.password = password;
  }

  /**
   * @return The port number of this object
   */
  public int getPort() {
    return port;
  }

  /**
   * @param port The port number this object should use.
   */
  public void setPort(int port) {
    this.port = port;
  }

  @Override
  public String toString() {
    return "(" + privateKeyStoreFile + ", '" + password + "', " + port + ")";
  }
}