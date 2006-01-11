// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:15:49
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   CupStructure.java

package Data;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.util.ArrayList;

// Referenced classes of package Data:
//            MatchList, Match, DeathMatch, WinnerOf, 
//            LoserOf, MatchPlacingInfo, Walkover, Player, 
//            Data, MatchClickListener, MatchDetails

public abstract class CupStructure extends MatchList
{
    class MatchListener
        implements MatchClickListener
    {

        public void matchClick(Match match, MouseEvent e)
        {
            if(match instanceof DeathMatch)
            {
                DeathMatch dm = (DeathMatch)match;
                if(matchDetails == null || match != matchDetails.getMatch())
                    showMatchDetails(dm, e.getPoint());
            }
        }

        MatchListener()
        {
        }
    }


    public abstract Match getMatch(int i);

    public abstract Dimension getDrawingSize();

    protected abstract Point getMatchPosition(Match match);

    public abstract int getNumRounds();

    public CupStructure()
    {
        doDetails = true;
        replayFinale = false;
        doDetails = true;
    }

    public Match getMatchAt(Point clicked)
    {
        int x = clicked.x;
        int y = clicked.y;
        Match selected = null;
        for(int i = 0; getMatch(i) != null; i++)
        {
            Point p = getMatchPosition(getMatch(i));
            if(x >= p.x && x < p.x + 132 && y >= p.y && y < p.y + 40)
                selected = getMatch(i);
        }

        return selected;
    }

    public boolean canBePlayed(Match ma)
    {
        DeathMatch m = (DeathMatch)ma;
        boolean ret = true;
        for(int i = 0; i <= 1; i++)
        {
            Player p = m.getPlayer(i);
            if(((p instanceof WinnerOf) || (p instanceof LoserOf)) && !m.getPlayer(i).getParentMatch().isPlayed())
                ret = false;
        }

        return ret;
    }

    public void drawHighlight(Point offset)
    {
        Graphics g = super.compoGraphics;
        if(g == null)
            return;
        if(super.highlightedMatch != null)
            drawMatch(g, offset, getMatchPosition(super.highlightedMatch), (DeathMatch)super.highlightedMatch);
        if(super.lastHighlighted != null)
            drawMatch(g, offset, getMatchPosition(super.lastHighlighted), (DeathMatch)super.lastHighlighted);
    }

    public void drawMatch(Graphics g, Point offset, Point p, DeathMatch m)
    {
        p.x += offset.x;
        p.y += offset.y;
        if(super.highlightedMatch == m)
            m.drawMatchHighlighted(p.x, p.y, g);
        else
            m.drawMatch(p.x, p.y, g);
        for(int i = 0; i < 2; i++)
        {
            if(m.getPlayer(i) instanceof WinnerOf)
            {
                Point pos = getMatchPosition(m.getPlayer(i).getParentMatch());
                if(pos == null)
                    continue;
                if(((Match) (m)).placingInfo.bracketCode == 4 && ((Match) (m)).placingInfo.roundIndex == 0 && i == 0)
                {
                    drawPathLine(offset.x + pos.x + 132 + 158, offset.y + pos.y + 20, p.x, p.y + 20, g);
                    drawPathLine(offset.x + pos.x + 132, offset.y + pos.y + 20, offset.x + pos.x + 132 + 158, offset.y + pos.y + 20, g);
                } else
                {
                    drawPathLine(offset.x + pos.x + 132, offset.y + pos.y + 20, p.x, p.y + 20, g);
                }
            }
            if(m.getPlayer(i) instanceof LoserOf)
            {
                MatchList.drawThickHLine(p.x - 10, p.y + (i * 40) / 2 + 10, p.x, p.y + (i * 40) / 2 + 10, g);
                MatchList.drawThickVLine(p.x - 10, p.y + (i * 40) / 2 + 10, p.x - 10, p.y + (i * 40) / 2, g);
            }
        }

    }

    protected void initFirstRound(Match match[], int roundLength)
    {
        super.data.shufflePlayerList(!super.data.doShuffle);
        for(int i = 0; i < roundLength; i++)
        {
            match[i] = new DeathMatch(String.valueOf(i + 1));
            match[i].placingInfo = new MatchPlacingInfo(0, roundLength, i, i, 0);
        }

        ArrayList players = super.data.getPlayerList();
        for(int i = players.size(); i < roundLength * 2; i++)
            players.add(new Walkover());

        matchNo = 0;
        match = splitPlayerList(players, match);
    }

    private Match[] splitPlayerList(ArrayList players, Match match[])
    {
        if(players.size() == 2)
        {
            match[matchNo].setPlayer(0, (Player)players.get(0));
            match[matchNo].setPlayer(1, (Player)players.get(1));
            matchNo++;
            return match;
        }
        ArrayList a = new ArrayList(players.size() / 2);
        ArrayList b = new ArrayList(players.size() / 2);
        for(int i = 0; i < players.size(); i++)
            if(((i + 1) / 2) % 2 == 0)
                a.add(players.get(i));
            else
                b.add(players.get(i));

        match = splitPlayerList(a, match);
        match = splitPlayerList(b, match);
        return match;
    }

    protected int getNumWalkovers()
    {
        return (int)Math.pow(2D, Math.ceil(Math.log(super.data.getNumPlayers()) / Math.log(2D))) - super.data.getNumPlayers();
    }

    protected void drawPathLine(int x1, int y1, int x2, int y2, Graphics g)
    {
        int c = (x1 + x2) / 2;
        y1--;
        y2--;
        c++;
        g.setColor(MatchList.LINE_COL1);
        g.drawLine(x1 - 2, y1, c, y1);
        g.drawLine(c, y1, c, y2);
        g.drawLine(c, y2, x2, y2);
        y1 += 2;
        y2 += 2;
        c -= 2;
        g.drawLine(x1 - 2, y1, c, y1);
        g.drawLine(c, y1, c, y2);
        g.drawLine(c, y2, x2, y2);
        y1--;
        y2--;
        c++;
        g.setColor(MatchList.LINE_COL2);
        g.drawLine(x1 - 2, y1, c, y1);
        g.drawLine(c, y1, c, y2);
        g.drawLine(c, y2, x2, y2);
    }

    public String findMatchId(Match target)
    {
        Match m;
        for(int i = 0; (m = getMatch(i)) != null; i++)
            if(m == target)
                return "".concat(String.valueOf(String.valueOf(i)));

        return null;
    }

    protected void drawWindow(Graphics g, Color col, String headline, int x, int y, int width, int height)
    {
        g.setColor(Color.black);
        g.drawRoundRect(x, y, width, height, 0, 0);
        g.setColor(col);
        g.fillRoundRect(x + 1, y + 1, width - 1, height - 1, 0, 0);
        if(headline != null)
            drawTextBox(g, HEADERBGCOL, headline, x + 10 + 5, y + 10 + 5, new Font(MatchList.getMainFont().getName(), 1, 16), 0, 0);
    }

    protected void drawTextBox(Graphics g, Color col, String text, int x, int y, int alignment)
    {
        Font font = new Font(MatchList.getMainFont().getName(), 1, 16);
        drawTextBox(g, col, text, x, y, font, alignment, 0);
    }

    protected void drawTextBox(Graphics g, Color col, String text, int x, int y, Font aFont, int alignment, 
            int roundness)
    {
        int height = 25;
        int padding = 5;
        if(aFont != null)
            g.setFont(aFont);
        FontMetrics fm = g.getFontMetrics();
        int strWidth = fm.stringWidth(text);
        if(alignment == 1)
            x -= strWidth + 2 * padding;
        g.setColor(Color.black);
        g.drawRoundRect(x, y, strWidth + padding * 2, height, roundness, roundness);
        g.setColor(col);
        g.fillRoundRect(x + 1, y + 1, (strWidth + padding * 2) - 1, height - 1, roundness, roundness);
        g.setColor(TEXTCOL);
        g.drawString(text, x + padding, y + 17);
        g.setFont(MatchList.getMainFont());
    }

    protected void initTransients()
    {
        super.initTransients();
        addMatchClickListener(new MatchListener());
    }

    public void mouseMoved(MouseEvent e)
    {
        super.mouseMoved(e);
        Point p = e.getPoint();
        DeathMatch m = (DeathMatch)getMatchAt(p);
        if(m != super.highlightedMatch)
        {
            super.lastHighlighted = super.highlightedMatch;
            super.highlightedMatch = m;
            drawHighlight(new Point(0, 0));
            if(super.myContainer != null)
                super.myContainer.repaint();
        }
    }

    private static final long serialVersionUID = 0xfbd071ebc6e21fc3L;
    public static final int XMARG = 5;
    public static final int YMARG = 10;
    protected static final int WIN_ROUNDNESS = 0;
    protected static final int WIN_MARGIN = 10;
    public static final int ALIGN_RIGHT = 1;
    public static final int ALIGN_LEFT = 0;
    public static final int roundInfoLines = 1;
    public static final int BBOXX = 158;
    public static final int BBOXY = 60;
    public static final int HEADERFONTSIZE = 16;
    public static final int HEADERFONTSTYLE = 1;
    public static final int HEADERHEIGHT = 20;
    public static final int TEXTHEIGHT = 10;
    public static Color HEADERBGCOL = new Color(255, 255, 255, 50);
    public static Color TEXTCOL = new Color(255, 255, 255);
    public static final int BRACKET_HEADER_WIDTH = 25;
    public static Color SECOL = new Color(90, 90, 90);
    public static Color WBCOL = new Color(87, 94, 88);
    public static Color LBCOL = new Color(110, 88, 70);
    public transient boolean doDetails;
    public int dmWinMode;
    public boolean teamMode;
    public int numFinales;
    public int numWalkovers;
    protected boolean replayFinale;
    private transient int matchNo;

}