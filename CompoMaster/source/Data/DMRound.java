/**

$Id: DMRound.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

package Data;

import java.io.Serializable;

public class DMRound
    implements Serializable
{

    public DMRound(int fragsA, int fragsB, String mapName)
    {
        frags = new int[2];
        frags[0] = fragsA;
        frags[1] = fragsB;
        this.mapName = mapName;
    }

    public DMRound(String mapName)
    {
        this(0, 0, mapName);
    }

    public int getScore(int playerIndex)
    {
        if(playerIndex > 1)
            return -1;
        else
            return frags[playerIndex];
    }

    public String getMapName()
    {
        return mapName;
    }

    protected String mapName;
    protected int frags[];
}