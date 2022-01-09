package com.company;

import javax.swing.*;
import java.awt.*;

public class Frame extends JFrame {

    public Frame() {
        this.setMinimumSize(new Dimension(400, 300));
        this.setApplicationPanel(new Welcome().getPanel());
        this.setTitle("Quiz.App");
        this.setDefaultCloseOperation(EXIT_ON_CLOSE);
        this.setVisible(true);

        this.addWindowListener(new java.awt.event.WindowAdapter() {
            @Override
            public void windowClosing(java.awt.event.WindowEvent windowEvent) {
                if(Application.connection != null)
                    Application.connection.close();
            }
        });
    }

    public void setApplicationPanel(JPanel applicationPanel) {
        this.getContentPane().removeAll();
        this.getContentPane().add(applicationPanel, BorderLayout.CENTER);
        this.revalidate();
        this.repaint();
        this.pack();
        this.setLocationRelativeTo(null);
    }
}
