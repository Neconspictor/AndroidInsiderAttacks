package de.unipassau.fim.reallife_security.demoapp.demoapp;

import android.os.AsyncTask;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.Closeable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ConnectException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.List;

import javax.net.SocketFactory;

/**
 * A task for sending a text message to a remote server.
 */
class SendTask extends AsyncTask<String, Void, String> {

    private Exception e;
    private List<OnMessageReceiveListener> listeners;
    private SocketFactory factory;
    private InetSocketAddress serverAddress;
    private int timeout;

    /**
     * Creates a new send task object.
     * @param listeners Listeners that should be informed when the server response is available.
     * @param factory Used to create a connection to the server.
     * @param serverAddress The server address
     * @param timeout max allowed time to wait while trying to establish a connection. Measured in
     *                milliseconds
     */
    public SendTask(List<OnMessageReceiveListener> listeners,
                    SocketFactory factory,
                    InetSocketAddress serverAddress,
                    int timeout) {
        super();
        this.listeners = listeners;
        this.factory = factory;
        this.serverAddress = serverAddress;
        this.timeout = timeout;
    }


    /**
     * Sends a message to the server and returns the server's response.
     * If an error occurs, the returned response will be null and the thrown exception is
     * stored in the private member 'e'
     * @param messages It is exepcted, that the first item contains the message to be send.
     * @return The response of the server or null if an error occurs.
     */
    protected String doInBackground(String... messages) {
        String message = messages[0];

        //create a connection to the server and send the data
        String response = null;
        BufferedReader reader = null;
        BufferedWriter writer = null;
        try (Socket socket = factory.createSocket()) {

            socket.connect(serverAddress, timeout);

            writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            writeMessage(message, writer);

            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            response = readResponse(reader);

            writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            writer.write("ok");
            writer.flush();

        } catch (IOException e) {
            this.e = e;
        } finally {
            closeSilently(reader);
            closeSilently(writer);
        }

        // Void cannot instantiated, so we must return null
        return response;
    }

    /**
     * Reads the response of the server.
     * @param reader the reader to get the response from.
     * @return The response of the server.
     * @throws IOException If any IO error occurs.
     */
    private String readResponse(BufferedReader reader) throws IOException {
        String response = null;

        String contentLengthLine = reader.readLine();
        int contentLength;
        try {
            contentLength = Integer.parseInt(contentLengthLine);
        } catch (NumberFormatException e) {
            throw new IOException("Couldn't retrieve content-length");
        }

        char[] buffer = new char[128];
        StringBuilder builder = new StringBuilder();

        while (contentLength > buffer.length) {
            int read = reader.read(buffer, 0, buffer.length);
            if (read != buffer.length) {
                throw new IOException("Couldn't retrieve whole response!");
            }

            builder.append(buffer, 0, read);

            contentLength -= buffer.length;
        }

        //finally one last buffer read
        int read = reader.read(buffer, 0, contentLength);
        if (read != contentLength) {
            throw new IOException("Couldn't retrieve whole response!");
        }

        builder.append(buffer, 0, read);
        response = builder.toString();

        return response;
    }

    /**
     * Sends a message to the server.
     * @param message The message to send
     * @param writer The writer that wraps of a socket connection.
     * @throws IOException If any IO error occurs.
     */
    private void writeMessage(String message, BufferedWriter writer) throws IOException {
        int contentLength = message.length();
        writer.write(String.valueOf(contentLength));
        writer.newLine();
        writer.write(message);
        writer.flush();
    }

    /**
     * Will be called, when the message was send and a response was collected from the server.
     * @param response The response message of the server or null, if a response couldn't be received
     */
    protected void onPostExecute(String response) {
        // TODO: check this.e
        // TODO: do something with the feed
        if (e != null && e instanceof ConnectException) {
            Log.e("SenTask", "Couldn't connect to server");
        }

        for (OnMessageReceiveListener listener : listeners) {
            listener.onReceive(response);
        }
    }

    /**
     * Closes a Closable and suppresses any thrown IO-Exception.
     * @param closeable The Closable to close.
     */
    private void closeSilently(Closeable closeable) {
        if (closeable == null) return;
        try {
            closeable.close();
        } catch (IOException e) {
        }
    }
}