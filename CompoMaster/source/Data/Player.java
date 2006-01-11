// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:17:09
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Player.java

package Data;

import java.awt.Color;
import java.io.Serializable;

// Referenced classes of package Data:
//            DeathMatch, Match, RealPlayer

public abstract class Player
    implements Serializable
{

    public Player()
    {
        rank = Data.MAXRANK;
    }

    public Match getParentMatch()
    {
        return null;
    }

    public abstract String getName();

    public int getRank()
    {
        return rank;
    }

    public void setRank(int r)
    {
        rank = r;
    }

    public boolean isDisabled()
    {
        return false;
    }

    public boolean isWalkover()
    {
        return false;
    }

    public RealPlayer getRealPlayer()
    {
        return null;
    }

    public Color getColor()
    {
        return DeathMatch.ABSTRACT_COL;
    }

    public String toString()
    {
        return getName();
    }

    private static final long serialVersionUID = 0x969ccefa2dea9111L;
    protected int rank;

}