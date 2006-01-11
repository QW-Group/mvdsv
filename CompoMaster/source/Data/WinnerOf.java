// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:17:30
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Player.java

package Data;

import java.awt.Color;
import java.io.PrintStream;
import java.io.Serializable;

// Referenced classes of package Data:
//            Player, DeathMatch, Data, Match

class WinnerOf extends Player
    implements Serializable
{

    public Match getParentMatch()
    {
        return parentMatch;
    }

    WinnerOf(Match m)
    {
        parentMatch = (DeathMatch)m;
    }

    public int getRank()
    {
        return 1;
    }

    private Player getParentWinner()
    {
        if(parentMatch == null)
            return null;
        else
            return parentMatch.getWinner();
    }

    public String getName()
    {
        if(Data.debug)
            System.out.println("    WINNER OF ".concat(String.valueOf(String.valueOf(parentMatch.getName()))));
        Player p;
        if((p = getParentWinner()) != null)
            return p.getName();
        else
            return "Winner ".concat(String.valueOf(String.valueOf(parentMatch.getName())));
    }

    public Color getColor()
    {
        Player p;
        if((p = getParentWinner()) == null)
            return DeathMatch.ABSTRACT_COL;
        else
            return p.getColor();
    }

    public boolean isWalkover()
    {
        if(Data.debug)
            System.out.println(String.valueOf(String.valueOf((new StringBuffer("      WNR-ISWALKOVER [")).append(getName()).append("]"))));
        Player p;
        if((p = getParentWinner()) == null)
            return false;
        else
            return p.isWalkover();
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(String.valueOf(String.valueOf((new StringBuffer("      WNR-ISDISABLED [")).append(getName()).append("]"))));
        Player p;
        if((p = getParentWinner()) == null)
            return false;
        else
            return p.isDisabled();
    }

    public String toString()
    {
        return "Winner of ".concat(String.valueOf(String.valueOf(parentMatch.getName())));
    }

    private static final long serialVersionUID = 0x289f5684b6dd3a8L;
    DeathMatch parentMatch;

}