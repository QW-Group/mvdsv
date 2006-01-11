// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:11:35
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   TextFieldLimiter.java

package CompoMaster;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.EventObject;
import javax.swing.JTextField;
import javax.swing.text.JTextComponent;

public class TextFieldLimiter extends KeyAdapter
{

    public TextFieldLimiter(int maxChar)
    {
        this(maxChar, false);
    }

    public TextFieldLimiter(int maxChar, boolean overwriteLast)
    {
        this.maxChar = maxChar;
        this.overwriteLast = overwriteLast;
    }

    public void keyTyped(KeyEvent e)
    {
        JTextField field = (JTextField)e.getSource();
        String text = field.getText();
        int flen = text.length();
        int selLen = field.getSelectedText() != null ? field.getSelectedText().length() : 0;
        if(flen - selLen >= maxChar)
            if(overwriteLast)
                field.setText(text.substring(0, maxChar - 1));
            else
            if(e.getKeyChar() != '\b')
            {
                field.setText(text.substring(0, maxChar - 1));
                e.setKeyChar(text.charAt(flen - 1));
            }
    }

    int maxChar;
    boolean overwriteLast;
}