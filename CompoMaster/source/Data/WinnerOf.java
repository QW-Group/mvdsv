/**

$Id: WinnerOf.java,v 1.2 2006/11/27 15:15:49 vvd0 Exp $

**/

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
            System.out.println("    WINNER OF ".concat(parentMatch.getName()));
        Player p;
        if((p = getParentWinner()) != null)
            return p.getName();
        else
            return "Winner ".concat(parentMatch.getName());
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
            System.out.println(new String((new StringBuffer("      WNR-ISWALKOVER [")).append(getName()).append("]")));
        Player p;
        if((p = getParentWinner()) == null)
            return false;
        else
            return p.isWalkover();
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(new String((new StringBuffer("      WNR-ISDISABLED [")).append(getName()).append("]")));
        Player p;
        if((p = getParentWinner()) == null)
            return false;
        else
            return p.isDisabled();
    }

    public String toString()
    {
        return "Winner of ".concat(parentMatch.getName());
    }

    private static final long serialVersionUID = 0x289f5684b6dd3a8L;
    DeathMatch parentMatch;

}