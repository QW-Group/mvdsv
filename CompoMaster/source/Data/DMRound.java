// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:18
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   DMRound.java

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