// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:09:49
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   JCompoDrawing.java

package CompoMaster;

import Data.Data;
import Data.MatchList;
import java.awt.Graphics;
import javax.swing.JComponent;
import javax.swing.JPanel;

public class JCompoDrawing extends JPanel
{

    public JCompoDrawing(Data data)
    {
        this.data = data;
        data.matchList.setMyContainer(this);
    }

    public void paintComponent(Graphics g)
    {
        super.paintComponent(g);
        g.drawImage(data.matchList.getImage(), 0, 0, this);
    }

    Data data;
    Graphics g;
}