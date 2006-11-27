/**

$Id: MatchPlayer.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

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