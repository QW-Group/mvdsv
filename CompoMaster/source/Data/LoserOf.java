// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:45
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Player.java

package Data;

import java.awt.Color;
import java.io.PrintStream;
import java.io.Serializable;

// Referenced classes of package Data:
//            Player, DeathMatch, Data, Match

class LoserOf extends Player
    implements Serializable
{

    LoserOf(Match m)
    {
        parentMatch = (DeathMatch)m;
    }

    public Match getParentMatch()
    {
        return parentMatch;
    }

    public int getRank()
    {
        return Data.MAXRANK;
    }

    private Player getParentLoser()
    {
        if(parentMatch == null)
            return null;
        else
            return parentMatch.getLoser();
    }

    public String getName()
    {
        if(Data.debug)
            System.out.println("    LOSER OF ".concat(String.valueOf(String.valueOf(parentMatch.getName()))));
        Player p;
        if((p = getParentLoser()) != null)
            return p.getName();
        else
            return "Loser ".concat(String.valueOf(String.valueOf(parentMatch.getName())));
    }

    public Color getColor()
    {
        Player p;
        if((p = getParentLoser()) == null)
            return DeathMatch.ABSTRACT_COL;
        else
            return p.getColor();
    }

    public boolean isWalkover()
    {
        if(Data.debug)
            System.out.println(String.valueOf(String.valueOf((new StringBuffer("      LSR-ISWALKOVER [")).append(getName()).append("]"))));
        Player p;
        if((p = getParentLoser()) == null)
            return false;
        else
            return p.isWalkover();
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(String.valueOf(String.valueOf((new StringBuffer("      LSR-ISDISABLED [")).append(getName()).append("]"))));
        Player p;
        if((p = getParentLoser()) == null)
            return false;
        else
            return p.isDisabled();
    }

    public String toString()
    {
        return "Loser of ".concat(String.valueOf(String.valueOf(parentMatch.getName())));
    }

    private static final long serialVersionUID = 0xe647d6f4244e9339L;
    DeathMatch parentMatch;

}