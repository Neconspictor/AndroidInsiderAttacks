import java.io.*;
import java.net.InetAddress;
import java.net.Socket;

public class Main {

    public static final int PORT = 5050;
    public static final String evilTestOutputFile = "./evilTestFile";

    public static void main(String[] args) throws IOException {
	    Socket socket = new Socket(InetAddress.getByName("localhost"), PORT);
	    try {
            InputStream in = socket.getInputStream();
            byte[] buffer = new byte[1024];
            try(OutputStream out = new FileOutputStream(evilTestOutputFile)) {

                for (int read = in.read(buffer, 0, buffer.length); read != -1; ) {
                    out.write(buffer, 0, read);
                    out.flush();
                    read = in.read(buffer, 0, buffer.length);
                }
            }
        } finally {
            closeSilently(socket);
        }
    }


    private static void closeSilently(Closeable closeable) {
        if (closeable == null) return;
        try {
            closeable.close();
        } catch (IOException e) {}
    }
}
