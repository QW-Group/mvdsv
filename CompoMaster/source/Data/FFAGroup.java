// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:31
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   FFAGroup.java

package Data;

import java.awt.FontMetrics;
import java.awt.Graphics;

// Referenced classes of package Data:
//            Group, MatchPlayer, RealPlayer, MatchList, 
//            Match, Player, Data

public class FFAGroup extends Group
{

    public FFAGroup(int num)
    {
        mPlayer = new MatchPlayer[num];
    }

    public boolean addPlayer(Player p)
    {
        for(int i = 0; i < mPlayer.length; i++)
            if(mPlayer[i] == null)
            {
                mPlayer[i] = new MatchPlayer(p);
                return true;
            }

        return false;
    }

    public int getGroupWidth()
    {
        return 240;
    }

    public void setScore(int pNr, int score)
    {
        mPlayer[pNr].score = score;
    }

    public int getScore(int pNr)
    {
        return mPlayer[pNr].score;
    }

    public void setPlayer(int pNr, Player p)
    {
        mPlayer[pNr] = new MatchPlayer(p);
    }

    public Player getPlayer(int nr)
    {
        return mPlayer[nr].player;
    }

    public Object getPlayerAt(int rank)
    {
        sortPlayers();
        if(--rank < 0 || rank >= mPlayer.length)
            return null;
        else
            return (RealPlayer)mPlayer[rank].player;
    }

    public int getNumPlayers()
    {
        return mPlayer.length;
    }

    public void sortPlayers()
    {
        for(int i = 0; i < mPlayer.length - 1; i++)
        {
            int j = i + 1;
            int largest = i;
            for(; j < mPlayer.length; j++)
                if(mPlayer[j].score > mPlayer[largest].score)
                    largest = j;

            MatchPlayer mp = mPlayer[i];
            mPlayer[i] = mPlayer[largest];
            mPlayer[largest] = mp;
        }

    }

    protected void drawMatchSub(int x, int y, Graphics g)
    {
        int origY = y;
        FontMetrics fm = g.getFontMetrics(MatchList.getMainFont());
        g.setColor(Group.headerCol);
        y += 15;
        g.drawString(super.name, x + 5, y);
        g.drawString("Scheduled: ", x + 5, y + 20);
        g.drawString("Frags:", ((x + 5) - 50) + getGroupWidth(), y + 20);
        g.setColor(Group.textCol);
        y += 20;
        if(!super.isPlayed)
        {
            g.drawString(super.scheduled, x + 5 + fm.stringWidth("Scheduled: "), y);
        } else
        {
            g.setColor(Group.greenTextCol);
            g.drawString("-Played-", x + 5 + fm.stringWidth("Scheduled: "), y);
        }
        y += 4;
        for(int j = 0; j < mPlayer.length; j++)
        {
            if(j % 2 == 0)
            {
                g.setColor(Group.BG1);
                g.fillRect(x + 2, y + 5, getGroupWidth() - 4, 20);
            }
            y += 20;
            g.setColor(Group.textCol);
            Data.drawName(g, mPlayer[j].player.getName(), x + 5, y - 2);
            g.setColor(Group.textCol);
            g.setColor(Group.textCol);
            if(super.isPlayed)
                MatchList.drawNumber(mPlayer[j].score, 3, ((x + 5) - 50) + getGroupWidth() + 18, y - 2, g);
        }

        y = origY;
        int groupHeight = getNumPlayers() * 20 + 44;
        g.drawLine((x + getGroupWidth()) - 50, y, (x + getGroupWidth()) - 50, y + groupHeight);
    }

    MatchPlayer mPlayer[];
}