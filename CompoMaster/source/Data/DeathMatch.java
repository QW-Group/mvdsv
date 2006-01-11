// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:12
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   DeathMatch.java

package Data;

import java.awt.*;
import java.io.PrintStream;
import java.io.Serializable;
import java.util.ArrayList;
import nilzorlib.diverse.ColorTools;

// Referenced classes of package Data:
//            Match, MatchPlayer, DMRound, Data, 
//            Player, MatchList

public class DeathMatch extends Match
    implements Serializable
{

    DeathMatch(Player p1, Player p2)
    {
        rounds = new ArrayList(3);
        highlight = false;
        woCode = -1;
        woFrags = 0;
        skipWoCheck = false;
        mPlayer = new MatchPlayer[2];
        mPlayer[0] = new MatchPlayer(p1);
        mPlayer[1] = new MatchPlayer(p2);
    }

    DeathMatch(Player p1, Player p2, String matchName)
    {
        super(matchName);
        rounds = new ArrayList(3);
        highlight = false;
        woCode = -1;
        woFrags = 0;
        skipWoCheck = false;
        mPlayer = new MatchPlayer[2];
        mPlayer[0] = new MatchPlayer(p1);
        mPlayer[1] = new MatchPlayer(p2);
    }

    DeathMatch(String matchName)
    {
        super(matchName);
        rounds = new ArrayList(3);
        highlight = false;
        woCode = -1;
        woFrags = 0;
        skipWoCheck = false;
        mPlayer = new MatchPlayer[2];
        mPlayer[0] = new MatchPlayer(null);
        mPlayer[1] = new MatchPlayer(null);
    }

    public Player getPlayer(int i)
    {
        return mPlayer[i].player;
    }

    public void setPlayer(int i, Player p)
    {
        mPlayer[i].player = p;
    }

    public int getScore(int i)
    {
        return mPlayer[i].score;
    }

    public void setScore(int i, int score)
    {
        mPlayer[i].score = score;
        skipWoCheck = false;
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(String.valueOf(String.valueOf((new StringBuffer("     MATCH-isDisabled [")).append(getName()).append("]= ").append(mPlayer[0].player.isDisabled() || mPlayer[1].player.isDisabled()))));
        return mPlayer[0].player.isDisabled() || mPlayer[1].player.isDisabled();
    }

    public Player getWinner()
    {
        if(!isPlayed())
            return null;
        int lsr = getWalkover();
        if(lsr != -1)
            return mPlayer[1 - lsr].player;
        if(mPlayer[0].score > mPlayer[1].score)
            return mPlayer[0].player;
        if(mPlayer[0].score == mPlayer[1].score)
            return null;
        else
            return mPlayer[1].player;
    }

    public Player getLoser()
    {
        if(!isPlayed())
            return null;
        int lsr = getWalkover();
        if(lsr != -1)
            return mPlayer[lsr].player;
        if(mPlayer[0].score > mPlayer[1].score)
            return mPlayer[1].player;
        if(mPlayer[0].score == mPlayer[1].score)
            return null;
        else
            return mPlayer[0].player;
    }

    public boolean isDraw()
    {
        return isPlayed() && mPlayer[0].score == mPlayer[1].score;
    }

    public boolean canBePlayed()
    {
        Match m;
        if((m = getPlayer(0).getParentMatch()) != null && !m.isPlayed())
            return false;
        return (m = getPlayer(1).getParentMatch()) == null || m.isPlayed();
    }

    public int getWalkover()
    {
        if(skipWoCheck)
            return woCode;
        if(mPlayer[0].player.isWalkover())
            woCode = 0;
        else
        if(mPlayer[1].player.isWalkover())
            woCode = 1;
        else
        if("!WALKOVER!".equals(getMap(0)))
        {
            if(getFrags(0, 0) == 1)
                woCode = 0;
            else
                woCode = 1;
        } else
        if("*WALKOVER_FRAGS".equals(getMap(0)) || "*WALKOVER".equals(getMap(0)))
        {
            if(getFrags(0, 0) > getFrags(0, 1))
                woCode = 1;
            else
                woCode = 0;
        } else
        {
            woCode = -1;
        }
        skipWoCheck = true;
        return woCode;
    }

    public boolean isPlayed()
    {
        if(getWalkover() != -1)
            return true;
        else
            return super.isPlayed;
    }

    public void setWalkover(int i)
    {
        setWalkover(i, 0);
    }

    public void setWalkover(int i, int frags)
    {
        setScore(0, 0);
        deleteRounds();
        woCode = i;
        woFrags = frags;
        super.isPlayed = true;
        skipWoCheck = true;
    }

    public void setIsPlayed(boolean is)
    {
        super.isPlayed = is;
        woCode = -1;
        skipWoCheck = false;
    }

    public int getNumPlayers()
    {
        return 2;
    }

    public void drawMatchHighlighted(int x, int y, Graphics g)
    {
        highlight = true;
        drawMatch(x, y, g);
        highlight = false;
    }

    public void drawMatch(int x, int y, Graphics g)
    {
        int ROUNDNESS = 6;
        int adjust = 9;
        Color c = mPlayer[0].player.getColor();
        if(getWalkover() == 0 && isPlayed())
            c = M_WO_COL;
        if(highlight)
            c = ColorTools.highlightColor(c, 50, 1, false);
        g.setColor(c);
        g.fillRoundRect(x + 0 + 1, y + 1, 131, 19, ROUNDNESS, ROUNDNESS);
        c = mPlayer[1].player.getColor();
        if(getWalkover() == 1 && isPlayed())
            c = M_WO_COL;
        if(highlight)
            c = ColorTools.highlightColor(c, 50, 1, false);
        g.setColor(c);
        g.fillRoundRect(x + 0 + 1, y + 20 + 1, 131, 19, ROUNDNESS, ROUNDNESS);
        g.setColor(M_FRAMECOL);
        g.drawRoundRect(x + 0 + 1, y, 130, 20, ROUNDNESS, ROUNDNESS * 2);
        g.drawRoundRect(x + 0 + 1, y + 20, 130, 20, ROUNDNESS, ROUNDNESS * 2);
        g.drawLine(x + 105, y, x + 105, y + 40);
        g.setFont(MatchList.getMainFont());
        g.setColor(IDCOLOR);
        g.drawString(super.name, x + 1, y - 4);
        g.setColor(M_TEXTCOL);
        Data.drawName(g, mPlayer[0].player.getName(), x + 0 + 4, (y + 4 + 20) - adjust);
        Data.drawName(g, mPlayer[1].player.getName(), x + 0 + 4, (y + 4 + 40) - adjust);
        g.setColor(M_TEXTCOL);
        String temp;
        String temp2;
        if(getWalkover() == 1)
        {
            temp = "WO";
            temp2 = "";
        } else
        if(getWalkover() == 0)
        {
            temp = "";
            temp2 = "WO";
        } else
        {
            temp = new String(String.valueOf(mPlayer[0].score));
            temp2 = new String(String.valueOf(mPlayer[1].score));
        }
        if(!isPlayed())
            temp = temp2 = "";
        for(int i = temp.length(); i < 2; i++)
            temp = " ".concat(String.valueOf(String.valueOf(temp)));

        for(int i = temp2.length(); i < 2; i++)
            temp2 = " ".concat(String.valueOf(String.valueOf(temp2)));

        g.drawString(temp, x + 105 + 4, (y + 4 + 20) - adjust);
        g.drawString(temp2, x + 105 + 4, (y + 4 + 40) - adjust);
    }

    public void addRound(DMRound newRound)
    {
        rounds.add(newRound);
    }

    public void setRound(DMRound newRound, int index)
    {
        rounds.set(index, newRound);
    }

    public void deleteRounds()
    {
        rounds = new ArrayList();
        setScore(0, 0);
    }

    public DMRound getRound(int idx)
    {
        return (DMRound)rounds.get(idx);
    }

    public int getNumRounds()
    {
        return rounds.size();
    }

    public void calculateScore()
    {
        mPlayer[0].score = mPlayer[1].score = 0;
        for(int i = 0; i < rounds.size(); i++)
        {
            DMRound r = (DMRound)rounds.get(i);
            if(r.getScore(0) > r.getScore(1))
                mPlayer[0].score++;
            if(r.getScore(1) > r.getScore(0))
                mPlayer[1].score++;
        }

    }

    public int getFrags(int rnd, int plr)
    {
        return ((DMRound)rounds.get(rnd)).getScore(plr);
    }

    public String getMap(int rnd)
    {
        if(rnd < 0 || rnd >= rounds.size())
        {
            return "";
        } else
        {
            DMRound r = (DMRound)rounds.get(rnd);
            return r.getMapName();
        }
    }

    public String toString()
    {
        String playCode = "A";
        if(canBePlayed())
            playCode = "B";
        if(isPlayed())
            playCode = "C";
        String str = "";
        for(int i = 0; i < rounds.size(); i++)
        {
            DMRound rptr = (DMRound)rounds.get(i);
            str = String.valueOf(str) + String.valueOf(String.valueOf(String.valueOf((new StringBuffer(":")).append(rptr.mapName).append(":").append(rptr.getScore(0)).append(":").append(rptr.getScore(1)))));
        }

        return String.valueOf(String.valueOf((new StringBuffer(String.valueOf(String.valueOf(super.name)))).append(":").append(playCode).append(':').append(getPlayer(0).getName()).append(':').append(getPlayer(1).getName()).append(':').append(getScore(0)).append(':').append(getScore(1)).append(':').append(super.scheduled).append(str)));
    }

    private static final String WALKOVER_OLD = "!WALKOVER!";
    private static final String WALKOVER = "*WALKOVER";
    private static final String WALKOVER_FRAGS = "*WALKOVER_FRAGS";
    public static final int HIGHLIGHTWEIGHT = 50;
    public static final int NO_WALKOVER = -1;
    private static final long serialVersionUID = 0xc664d5b8fad5d9d4L;
    public static Color M_BGCOL1 = new Color(100, 30, 30);
    public static Color ABSTRACT_COL = new Color(60, 20, 0);
    public static final Color M_DROPPERCOL = new Color(240, 240, 200);
    public static Color M_TEXTCOL = new Color(255, 255, 200);
    public static Color M_WO_COL = new Color(70, 70, 90);
    public static Color M_FRAMECOL = new Color(255, 255, 255);
    public static Color INFO_TEXTCOL = new Color(255, 255, 255);
    public static Color IDCOLOR = new Color(255, 255, 200);
    public static final Color HLCOL = new Color(100, 100, 100);
    public static final int BOXWIDTH = 132;
    public static final int BOXHEIGHT = 40;
    public static final int BOXX1 = 0;
    public static final int BOXX2 = 105;
    public static final int CP = 4;
    public static final int BOXSPACEX = 26;
    public static final int BOXSPACEY = 20;
    public static final int MAXDIGITS = 2;
    public static Point detailedDrawPoint = null;
    public static boolean editHighlight = false;
    private MatchPlayer mPlayer[];
    private ArrayList rounds;
    private transient boolean highlight;
    private int woCode;
    private int woFrags;
    private boolean skipWoCheck;
    private transient Data data;

}