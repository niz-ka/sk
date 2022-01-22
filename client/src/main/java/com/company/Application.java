package com.company;

import javax.swing.*;

public class Application {

    public static Frame frame;
    public static Connection connection;

    public static void main(String[] args) {
        frame = new Frame();

        try {
            connection = new Connection("127.0.0.1", 5050);
        } catch(java.io.IOException ex) {
            JOptionPane.showMessageDialog(
                    null,
                    "Błąd połączenia z serwerem. Proszę spróbować później.",
                    "Błąd",
                    JOptionPane.ERROR_MESSAGE);
        }
    }
}
