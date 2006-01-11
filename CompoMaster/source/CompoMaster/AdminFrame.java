// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:06:16
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   AdminFrame.java

package CompoMaster;

import Data.*;
import com.borland.jbcl.layout.XYConstraints;
import com.borland.jbcl.layout.XYLayout;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

// Referenced classes of package CompoMaster:
//            Menu, JCompoDrawing, HtmlCode, ScoreRegDM, 
//            ScoreRegFFA, CompoMaster

public class AdminFrame extends JFrame
    implements MatchClickListener
{
    class WinEvents extends WindowAdapter
        implements ComponentListener
    {

        public void windowClosing(WindowEvent e)
        {
            int result = JOptionPane.showConfirmDialog(CompoMaster.activeFrame, "Save before quitting?", "Quit program", 0);
            if(result == 0)
                CompoMaster.saveCompo();
            if(result != 2)
                CompoMaster.exit();
            else
                CompoMaster.activeFrame.show();
        }

        public void componentHidden(ComponentEvent componentevent)
        {
        }

        public void componentMoved(ComponentEvent componentevent)
        {
        }

        public void componentResized(ComponentEvent e)
        {
            adjustScrollPane();
        }

        public void componentShown(ComponentEvent componentevent)
        {
        }

        WinEvents()
        {
        }
    }


    public AdminFrame(String s, Data d)
    {
        super(s);
        xYLayout1 = new XYLayout();
        String title = "CompoMaster";
        if(CompoMaster.dataFileName != null)
            title = String.valueOf(title) + String.valueOf(" - ".concat(String.valueOf(String.valueOf(CompoMaster.dataFileName))));
        setTitle(title);
        CompoMaster.setDataInstance(d);
        data = d;
        jbInit();
    }

    public void jbInit()
    {
        setIconImage(CompoMaster.getIcon());
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        screenSize.width -= 110;
        screenSize.height -= 110;
        Dimension drawingSize = data.matchList.getDrawingSize();
        Dimension frameSize = new Dimension();
        if(screenSize.width > drawingSize.width + 12)
            frameSize.width = drawingSize.width + 12;
        else
            frameSize.width = screenSize.width;
        if(screenSize.height > drawingSize.height + 12 + 25)
            frameSize.height = drawingSize.height + 12 + 25;
        else
            frameSize.height = screenSize.height;
        frameSize.width += 10;
        frameSize.height += 20;
        setSize(frameSize);
        getContentPane().setLayout(xYLayout1);
        theMenu = new Menu(data, true);
        drawing = new JCompoDrawing(data);
        drawing.setLayout(new XYLayout(drawingSize.width, drawingSize.height));
        drawing.setSize(drawingSize);
        sp = new JScrollPane(drawing);
        Dimension spSize = adjustScrollPane();
        getContentPane().add(theMenu, new XYConstraints(0, 0, 330, 24));
        getContentPane().add(sp, new XYConstraints(4, 25, spSize.width, spSize.height));
        adjustScrollPane();
        WinEvents winListener = new WinEvents();
        addComponentListener(winListener);
        addWindowListener(winListener);
        data.matchList.addMatchClickListener(this);
        setLocation(55, 55);
    }

    public static void showHtml()
    {
        if(CompoMaster.dataFileName == null)
        {
            JOptionPane.showMessageDialog(CompoMaster.activeFrame, "You must save and give the tournament a filename before you can prepare an HTML page.", "HTML generator", 0);
        } else
        {
            HtmlCode c = new HtmlCode(CompoMaster.getDataInstance());
            c.show();
        }
    }

    public Dimension adjustScrollPane()
    {
        Dimension frameSize = getSize();
        Dimension spSize = new Dimension();
        spSize.width = (frameSize.width - 12) + 2;
        spSize.height = frameSize.height - 12 - 25 - 15;
        sp.setSize(spSize);
        sp.doLayout();
        doLayout();
        return spSize;
    }

    public void matchClick(Match match, MouseEvent e)
    {
        if(match instanceof DeathMatch)
        {
            ScoreRegDM regFrame = new ScoreRegDM((DeathMatch)match, this);
            regFrame.pack();
            regFrame.show();
        }
        if(match instanceof FFAGroup)
        {
            ScoreRegFFA regFrame = new ScoreRegFFA((FFAGroup)match, this);
            regFrame.pack();
            regFrame.show();
        }
    }

    public final int MARGIN = 4;
    public final int MENUHEIGHT = 25;
    public static final int SCREEN_MARGIN = 55;
    public static Menu theMenu;
    private JCompoDrawing drawing;
    XYLayout xYLayout1;
    JScrollPane sp;
    private Data data;

}