/**

$Id: CompoSelecter.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

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
                text = new String((new StringBuffer("Tournament ")).append(i + 1));
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