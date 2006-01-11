// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:13:00
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   CompoDrawing.java

package CompoViewer;

import Data.*;
import com.borland.jbcl.layout.XYLayout;
import java.awt.*;

// Referenced classes of package CompoViewer:
//            CompoViewer

public class CompoDrawing extends Panel
{

    public CompoDrawing(Data data, Dimension size)
    {
        firstTime = true;
        up = 0;
        pa = 0;
        this.data = data;
        offset = new Point(0, 0);
        dim = data.matchList.getSize();
        setSize(dim);
        setLayout(new XYLayout());
        setBackground(CompoViewer.bgColor);
        data.matchList.setMyContainer(this);
        data.matchList.setBgcolor(CompoViewer.bgColor);
        if(data.matchList instanceof CupStructure)
            ((CupStructure)data.matchList).doDetails = true;
    }

    public void paint(Graphics g)
    {
        if(CompoViewer.applet != null)
            CompoViewer.applet.debugPrint("Paint start");
        g.drawImage(data.matchList.getImage(), 0, 0, this);
        if(CompoViewer.applet != null)
            CompoViewer.applet.debugPrint("Paint end");
        if(CompoViewer.nagOn)
        {
            int height = getSize().height;
            g.setColor(Color.green);
            g.setFont(new Font("", 1, 24));
            g.drawString("Non-public test release. Copying prohibited.", 10, height / 2);
        }
    }

    public void update(Graphics g)
    {
        paint(g);
    }

    public Point getOffset()
    {
        return offset;
    }

    public Dimension getSize()
    {
        return dim;
    }

    public Dimension getPreferredSize()
    {
        return dim;
    }

    public Dimension getMinimumSize()
    {
        return dim;
    }

    public Data data;
    private Point offset;
    private Dimension dim;
    private boolean firstTime;
    private int up;
    private int pa;
}