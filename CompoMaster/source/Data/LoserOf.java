/**

$Id: LoserOf.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

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
            System.out.println("    LOSER OF ".concat(parentMatch.getName()));
        Player p;
        if((p = getParentLoser()) != null)
            return p.getName();
        else
            return "Loser ".concat(parentMatch.getName());
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
            System.out.println(new String((new StringBuffer("      LSR-ISWALKOVER [")).append(getName()).append("]")));
        Player p;
        if((p = getParentLoser()) == null)
            return false;
        else
            return p.isWalkover();
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(new String((new StringBuffer("      LSR-ISDISABLED [")).append(getName()).append("]")));
        Player p;
        if((p = getParentLoser()) == null)
            return false;
        else
            return p.isDisabled();
    }

    public String toString()
    {
        return "Loser of ".concat(parentMatch.getName());
    }

    private static final long serialVersionUID = 0xe647d6f4244e9339L;
    DeathMatch parentMatch;

}
