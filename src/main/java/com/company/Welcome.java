package com.company;

import javax.swing.*;

public class Welcome {
    private JTextArea codeTextArea;
    private JButton joinButton;
    private JButton createButton;
    private JPanel panel;

    public Welcome() {
        createButton.addActionListener(actionEvent -> {
            if(Application.connection != null)
                Application.connection.send("MAKE");
        });
    }

    public JPanel getPanel() {
        return this.panel;
    }
}
