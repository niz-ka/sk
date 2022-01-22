package com.company;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.net.Socket;

public class Connection {
    private final static int HEADER = 4;

    private final Socket socket;
    private final DataOutputStream out;
    private final DataInputStream in;

    public Connection(String address, int port) throws java.io.IOException {
        socket = new Socket(address, port);
        out = new DataOutputStream(socket.getOutputStream());
        in = new DataInputStream(socket.getInputStream());
    }

    public void close() {
        try {
            in.close();
            out.close();
            socket.close();
        } catch(java.io.IOException ex) {
            System.out.println("[Exception] close()" + ex);
        }
    }

    public void send(String message) {
        char[] lengthArray = new char[HEADER];
        char[] tempArray = String.valueOf(message.length()).toCharArray();

        System.arraycopy(tempArray, 0, lengthArray, 0, tempArray.length);

        message = String.valueOf(lengthArray) + message;
        System.out.println(message);
        try {
            out.write(message.getBytes());
        }
        catch(java.io.IOException ex) {
            System.out.println("[Exception] write()" + ex);
        }
    }
}
