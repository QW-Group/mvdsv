// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:35
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Group.java

package Data;

import java.awt.*;
import java.io.Serializable;
import nilzorlib.diverse.ColorTools;

// Referenced classes of package Data:
//            Match, Player, Rankable, MatchList

public abstract class Group extends Match
    implements Rankable, Serializable
{

    public Group()
    {
    }

    protected abstract void drawMatchSub(int i, int j, Graphics g);

    public abstract boolean addPlayer(Player player);

    public abstract int getGroupWidth();

    public String stringStandings()
    {
        return null;
    }

    public Dimension drawMatchInfo(Graphics g, Point p, boolean a)
    {
        return null;
    }

    public String getRankText(int rank)
    {
        return String.valueOf(String.valueOf((new StringBuffer("No. ")).append(rank).append(", ").append(getName())));
    }

    public void drawMatch(int x, int y, Graphics g)
    {
        int groupHeight = getNumPlayers() * 20 + 44;
        g.setColor(ColorTools.highlightColor(BG1, 30, 0, false));
        g.fillRect(x, y, getGroupWidth(), groupHeight);
        xPos = x;
        yPos = y;
        drawMatchSub(x, y, g);
        MatchList.drawThickHLine(x, y, x + getGroupWidth(), y, g);
        MatchList.drawThickVLine(x, y + groupHeight, x + getGroupWidth(), y + groupHeight, g);
        MatchList.drawThickVLine(x, y, x, y + groupHeight, g);
        MatchList.drawThickVLine(x + getGroupWidth(), y, x + getGroupWidth(), y + groupHeight, g);
        MatchList.drawThickVLine(x, -1 + y + 44, x + getGroupWidth(), -1 + y + 44, g);
        g.setColor(MatchList.LINE_COL1);
    }

    public abstract Object getPlayerAt(int i);

    public int xPos;
    public int yPos;
    private static final long serialVersionUID = 0xc0c71dc179bc2c3cL;
    public static final int SCOREWIDTH = 50;
    public static final int HIGHLIGHTWEIGHT = 30;
    public static Color greenTextCol = new Color(0, 255, 0);
    public static Color headerCol = new Color(240, 240, 180);
    public static Color advanceCol = new Color(30, 150, 30);
    public static Color maybeAdvanceCol = new Color(200, 200, 30);
    public static Color highlightCol = new Color(135, 95, 20);
    public static Color disabledCol = new Color(50, 50, 50);
    public static final int FFAGROUPWIDTH = 240;
    public static final int LEAGUEGROUPWIDTH = 376;
    public static Color BG1 = new Color(100, 30, 30);
    public static final Color textCol = new Color(240, 240, 240);
    public static final int LA = 20;
    public static final int XPAD = 5;
    public static final int YPAD = 5;
    public static final int XMARG = 10;
    public static final int YMARG = 10;
    public static final int HEADERHEIGHT = 44;
    protected static Player markedPlayer[] = new Player[2];
    protected static Group markedGroup = null;

}