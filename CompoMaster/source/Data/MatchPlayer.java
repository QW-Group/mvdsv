// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:57
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   MatchPlayer.java

package Data;

import java.io.Serializable;

// Referenced classes of package Data:
//            Player

public class MatchPlayer
    implements Serializable
{

    MatchPlayer(Player p)
    {
        player = p;
        score = 0;
    }

    int score;
    Player player;
}