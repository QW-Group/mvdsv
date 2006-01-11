// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:13:04
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   CompoSelecter.java

package CompoViewer;

import Data.Data;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.EventObject;

// Referenced classes of package CompoViewer:
//            CompoViewer

public class CompoSelecter extends Panel
    implements ActionListener
{

    public int getWidth()
    {
        return 2000;
    }

    public int getHeight()
    {
        return 30;
    }

    public void actionPerformed(ActionEvent e)
    {
        for(int i = 0; i < 10; i++)
            if(button[i] != null && button[i] == e.getSource())
                c.setSelected(i);

    }

    public CompoSelecter(CompoViewer c)
    {
        textCol = new Color(250, 250, 240);
        highlightCol = new Color(100, 100, 100);
        name = new String[10];
        setLayout(new FlowLayout(0, 5, 0));
        setSize(c.getSize().width, 30);
        this.c = c;
        button = new Button[10];
        for(int i = 0; c.getData(i) != null; i++)
        {
            String text = c.getData(i).getCompoName();
            if(text == null || text.length() == 0)
                text = String.valueOf(String.valueOf((new StringBuffer("Tournament ")).append(i + 1)));
            button[i] = new Button(text);
            button[i].addActionListener(this);
            add(button[i]);
        }

    }

    public static final int HEIGHT = 30;
    private Color textCol;
    private Color highlightCol;
    String name[];
    private CompoViewer c;
    Button button[];

}