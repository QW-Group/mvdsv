// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:53
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   MatchList.java

package Data;

import com.borland.jbcl.layout.XYConstraints;
import java.awt.*;
import java.awt.event.*;
import java.io.Serializable;
import java.util.ArrayList;

// Referenced classes of package Data:
//            MatchDetails, MatchClickListener, Match, Data, 
//            EditListener, DeathMatch

public abstract class MatchList
    implements Serializable, MouseListener, MouseMotionListener
{

    public MatchList()
    {
        editDirectly = false;
        myContainer = null;
        bgcolor = Color.black;
    }

    protected abstract void drawCompo(Point point);

    public abstract Dimension getDrawingSize();

    public abstract void initCompo();

    public abstract Match getMatch(int i);

    public abstract Match getMatchAt(Point point);

    public abstract boolean canBePlayed(Match match);

    public abstract String findMatchId(Match match);

    public void setHighlighted(Match m)
    {
        highlightedMatch = m;
    }

    public Match getHighlighted()
    {
        return highlightedMatch;
    }

    public int getNumMatches()
    {
        return numMatches;
    }

    public void setBgcolor(Color c)
    {
        bgcolor = c;
    }

    public Color getBgcolor()
    {
        return bgcolor;
    }

    public void setMyContainer(Container c)
    {
        myContainer = c;
        initTransients();
    }

    protected void initTransients()
    {
        if(matchClickListeners == null)
            matchClickListeners = new ArrayList();
        bgcolor = Color.black;
        myContainer.addMouseListener(this);
        myContainer.addMouseMotionListener(this);
    }

    public void updateImage()
    {
        drawCompo(new Point(0, 0));
    }

    public void reset()
    {
        compoCanvas = null;
        lastHighlighted = null;
        highlightedMatch = null;
        editListener = null;
        matchClickListeners = new ArrayList();
        MatchDetails matchDetails = null;
        Container myContainer = null;
        Dimension imageSize = null;
    }

    public Image getImage()
    {
        if(compoCanvas == null)
        {
            if(myContainer == null)
                return null;
            imageSize = getDrawingSize();
            compoCanvas = myContainer.createImage(imageSize.width, imageSize.height);
            compoGraphics = compoCanvas.getGraphics();
            drawCompo(new Point(0, 0));
        }
        return compoCanvas;
    }

    protected void clearImage()
    {
        compoGraphics.setColor(bgcolor);
        compoGraphics.fillRect(0, 0, imageSize.width, imageSize.height);
    }

    public static void drawNumber(int nr, int digits, int x, int y, Graphics g)
    {
        FontMetrics fm = g.getFontMetrics(getMainFont());
        int width = fm.charWidth('0');
        String number = String.valueOf(nr);
        g.drawString(number, x + (digits - number.length()) * width, y);
    }

    public static void drawThickHLine(int x1, int y1, int x2, int y2, Graphics g)
    {
        g.setColor(LINE_COL1);
        g.drawLine(x1, y1, x2, y2);
        g.setColor(LINE_COL2);
        g.drawLine(x1, y1 + 1, x2, y2 + 1);
    }

    public static void drawThickVLine(int x1, int y1, int x2, int y2, Graphics g)
    {
        g.setColor(LINE_COL1);
        g.drawLine(x1, y1, x2, y2);
        g.setColor(LINE_COL2);
        g.drawLine(x1 + 1, y1, x2 + 1, y2);
    }

    public static Font getMainFont()
    {
        if(mainFont == null)
            mainFont = new Font("Courier", 0, 11);
        return mainFont;
    }

    public void setEditDirectly(boolean value)
    {
        editDirectly = value;
    }

    public void setEditListener(EditListener e)
    {
        editListener = e;
    }

    protected void drawVersionInfo(Graphics g)
    {
        Dimension di = getDrawingSize();
        if(di != null)
        {
            g.setColor(new Color(128, 128, 128));
            MatchList _tmp = this;
            String line = String.valueOf(String.valueOf((new StringBuffer(String.valueOf(String.valueOf(Data.VERSION_2)))).append(" v").append(Data.VERSION_NO)));
            g.drawString(line, di.width - g.getFontMetrics(getMainFont()).stringWidth(line) - 5, di.height - 6);
        }
    }

    public Dimension getSize()
    {
        if(imageSize != null)
            return imageSize;
        else
            return getDrawingSize();
    }

    public Dimension getPreferredSize()
    {
        return getSize();
    }

    protected void showMatchDetails(DeathMatch newMatch, Point drawPoint)
    {
        DeathMatch oldMatch;
        if(matchDetails == null)
            oldMatch = null;
        else
            oldMatch = matchDetails.getMatch();
        boolean changes = false;
        if(oldMatch != null && newMatch != oldMatch)
        {
            myContainer.remove(matchDetails);
            matchDetails = null;
            changes = true;
        }
        if(newMatch != null && newMatch != oldMatch)
        {
            matchDetails = new MatchDetails(newMatch, myContainer, editListener, data);
            Dimension d = matchDetails.getDimension();
            if(myContainer instanceof Panel)
                myContainer.add(matchDetails, new XYConstraints(drawPoint.x, drawPoint.y, d.width, d.height));
            changes = true;
        }
        if(changes)
        {
            myContainer.validate();
            myContainer.repaint();
        }
    }

    public void addMatchClickListener(MatchClickListener l)
    {
        if(matchClickListeners == null)
            matchClickListeners = new ArrayList();
        matchClickListeners.add(l);
    }

    public void removeMatchClickListener(MatchClickListener l)
    {
        int size = matchClickListeners.size();
        int i = 0;
        do
        {
            if(i >= size)
                break;
            Object o = matchClickListeners.get(i);
            if(o == l)
            {
                matchClickListeners.remove(i);
                break;
            }
            i++;
        } while(true);
    }

    protected void dispatchMatchClickEvents(Match m, MouseEvent e)
    {
        int size = matchClickListeners.size();
        for(int i = 0; i < size; i++)
            ((MatchClickListener)matchClickListeners.get(i)).matchClick(m, e);

    }

    public void mousePressed(MouseEvent e)
    {
        Point p = e.getPoint();
        Match m = getMatchAt(p);
        if(m != null)
            dispatchMatchClickEvents(m, e);
    }

    public void mouseReleased(MouseEvent mouseevent)
    {
    }

    public void mouseClicked(MouseEvent mouseevent)
    {
    }

    public void mouseMoved(MouseEvent mouseevent)
    {
    }

    public void mouseDragged(MouseEvent mouseevent)
    {
    }

    public void mouseEntered(MouseEvent mouseevent)
    {
    }

    public void mouseExited(MouseEvent mouseevent)
    {
    }

    private static final long serialVersionUID = 0x71d1070fcd806f47L;
    public static final Color LINE_COL1 = new Color(180, 180, 180);
    public static final Color LINE_COL2 = new Color(220, 220, 220);
    public static final String FONTTYPE = "Courier";
    public static final int FONTSIZE = 11;
    public static Font mainFont = null;
    public int numMatches;
    protected Data data;
    protected transient boolean editDirectly;
    protected transient Graphics compoGraphics;
    protected Match highlightedMatch;
    protected transient Match lastHighlighted;
    private transient EditListener editListener;
    private transient ArrayList matchClickListeners;
    protected transient MatchDetails matchDetails;
    protected transient Container myContainer;
    protected transient Dimension imageSize;
    protected transient Color bgcolor;
    protected transient Image compoCanvas;

}