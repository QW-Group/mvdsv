/**

$Id: RankOf.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.Color;
import java.io.PrintStream;

// Referenced classes of package Data:
//            Player, RealPlayer, Rankable, DeathMatch, 
//            Data, Match

class RankOf extends Player
{

    RankOf(Rankable source, int rank)
    {
        groupIdx = -1;
        this.source = source;
        super.rank = rank;
    }

    public void setGroupIndex(int i)
    {
        groupIdx = i;
    }

    public int getGroupIndex()
    {
        return groupIdx;
    }

    public void setSource(Rankable r)
    {
        source = r;
    }

    public Match getParentMatch()
    {
        return null;
    }

    public RealPlayer getRealPlayer()
    {
        if(source == null)
            return null;
        else
            return (RealPlayer)source.getPlayerAt(super.rank);
    }

    public Color getColor()
    {
        if(source != null && source.getPlayerAt(super.rank) != null)
            return getRealPlayer().getColor();
        else
            return DeathMatch.ABSTRACT_COL;
    }

    public boolean isWalkover()
    {
        if(Data.debug)
            System.out.println(new String((new StringBuffer("      RNK-ISWALKOVER [")).append(getName()).append("]")));
        if(source != null)
        {
            RealPlayer p = getRealPlayer();
            if(p != null)
                return p.isWalkover();
        }
        return false;
    }

    public boolean isDisabled()
    {
        if(Data.debug)
            System.out.println(new String((new StringBuffer("      RNK-ISDISABLED [")).append(getName()).append("]")));
        if(source != null)
        {
            RealPlayer p = getRealPlayer();
            if(p != null)
                return p.isDisabled();
        }
        return false;
    }

    public String getName()
    {
        if(Data.debug)
            System.out.println("    RANK OF -R=".concat(String.valueOf(super.rank)));
        if(source == null)
            return "(Broken link)";
        RealPlayer p = (RealPlayer)source.getPlayerAt(super.rank);
        if(p != null)
            return p.getName();
        else
            return source.getRankText(super.rank);
    }

    public String toString()
    {
        return getName();
    }

    private static final long serialVersionUID = 0xd4de715da7ed56b5L;
    private transient Rankable source;
    private int groupIdx;

}