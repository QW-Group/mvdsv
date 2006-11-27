/**

$Id: DoubleRound.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

package Data;


// Referenced classes of package Data:
//            DMRound

public class DoubleRound extends DMRound
{

    public DoubleRound(int fragsA1, int fragsB1, int fragsA2, int fragsB2, int sideA1, String mapName)
    {
        super(mapName);
        score = new int[2][2];
        score[0][0] = fragsA1;
        score[0][1] = fragsB1;
        score[1][0] = fragsA2;
        score[1][1] = fragsB2;
        sideIdx = sideA1;
    }

    public DoubleRound(String mapName)
    {
        super(mapName);
        score = new int[2][2];
        score[0][0] = 0;
        score[0][1] = 0;
        score[1][0] = 0;
        score[1][1] = 0;
        sideIdx = 0;
    }

    public int getScore(int team)
    {
        return score[0][team] + score[1][team];
    }

    public int getScore(int subround, int team)
    {
        return score[subround][team];
    }

    public int getSideIndex(int subround, int team)
    {
        switch(subround * 2 + team)
        {
        case 0: // '\0'
        case 3: // '\003'
            return sideIdx;

        case 1: // '\001'
        case 2: // '\002'
            return 1 - sideIdx;
        }
        return -1;
    }

    private int sideIdx;
    private int score[][];
}