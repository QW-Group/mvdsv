/**

$Id: Player.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

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