/**

$Id: MatchPlacingInfo.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.io.Serializable;

public class MatchPlacingInfo
    implements Serializable
{

    public MatchPlacingInfo()
    {
        roundIndex = 0;
        roundSize = 0;
        roundNr = 0;
        bracketCode = 0;
    }

    public MatchPlacingInfo(int roundNr, int roundSize, int roundIndex, int bracketIndex, int bracketCode)
    {
        this.roundIndex = roundIndex;
        this.roundSize = roundSize;
        this.roundNr = roundNr;
        this.bracketCode = bracketCode;
        this.bracketIndex = bracketIndex;
    }

    public boolean isInLB(int bracketCode)
    {
        return bracketCode != 2 && bracketCode != 3;
    }

    private static final long serialVersionUID = 0x4f848ce65177e526L;
    public static final int FIRSTROUND = 0;
    public static final int WB = 1;
    public static final int LB_A = 2;
    public static final int LB_B = 3;
    public static final int FDE_FINAL = 4;
    public static final int SE_FINAL = 5;
    public int bracketIndex;
    public int roundIndex;
    public int roundSize;
    public int roundNr;
    public int bracketCode;

}