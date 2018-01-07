import java.io.*;
import java.util.StringTokenizer;

public class Main {

    public static void main(String[] args) {
	// write your code here

        if (args.length != 2) {
            printUsage();
            return;
        }

        String sourceFile = args[0];
        String outputFile = args[1];


        try(BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(sourceFile)))) {
            processFile(reader, outputFile);
        } catch (FileNotFoundException e) {
            System.err.println("Couldn't open file: " + sourceFile);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void processFile(BufferedReader reader, String outFilePath) throws IOException {

        try(PrintWriter writer = new PrintWriter(new FileOutputStream(outFilePath))) {

            writer.println("unsigned char jniPayload[] = {");

            StringBuilder builder = new StringBuilder();

            String line = reader.readLine();
            while(line != null) {
                StringTokenizer tokenizer = new StringTokenizer(line);

                // discard the memory line number
                validate(getToken(tokenizer));

                //get the machine code as a string
                String machineCode = getToken(tokenizer);
                validate(machineCode);
                String outputLine = createByteLine(machineCode);

                String comment = getComment(tokenizer);
                validate(comment);
                comment = comment.trim();

                outputLine += " // " + comment + "\r\n";
                builder.append(outputLine);

                line = reader.readLine();
            }

            //delete last ', \n'
            String removeToken = ", \r\n";
            //if (builder.length() >= removeToken.length())
            //    builder.delete(builder.length() - removeToken.length(), builder.length());

            writer.println(builder.toString());
            writer.println("};");

        } catch (FileNotFoundException e) {
            System.err.println("Couldn't open output file: " + outFilePath);
        }
    }

    private static String getComment(StringTokenizer tokenizer) {
        if (tokenizer.hasMoreTokens()) {
            return tokenizer.nextToken("");
        }
        return null;

    }

    private static String createByteLine(String machineCode) {
        if (machineCode.length() % 2 != 0) { //the machine code has to be a multiple of 2 (a byte in hexadecimal form)
            return null;
        }

        StringBuilder builder = new StringBuilder();

        for (int i = 0; i < machineCode.length(); i +=2) {
            char hexa1 = machineCode.charAt(i);
            char hexa2 = machineCode.charAt(i+1);
            builder.append("0x" + hexa1);
            builder.append(hexa2 + ", ");

        }

        return builder.toString();
    }

    private static void validate(String str) throws IOException {
        if (str == null) {
            throw new IOException("Token mustn't be null");
        }
    }

    private static String getToken(StringTokenizer tokenizer) {
        if (tokenizer.hasMoreTokens()) return tokenizer.nextToken();
        return null;
    }


    private static void printUsage() {
        System.err.println("Usage: payloadBuilder <source-file> <output-file>");
    }
}