/**

$Id: Match.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.Graphics;
import java.io.Serializable;
import java.util.ArrayList;

// Referenced classes of package Data:
//            Demo, MatchPlacingInfo, Data, Player

public abstract class Match
    implements Serializable
{

    public abstract void setPlayer(int i, Player player);

    public abstract Player getPlayer(int i);

    public abstract int getNumPlayers();

    public abstract void drawMatch(int i, int j, Graphics g);

    public Match()
    {
        isPlayed = false;
        scheduled = new String("N/A");
        demoList = new ArrayList(3);
    }

    public Match(String name)
    {
        isPlayed = false;
        scheduled = new String("N/A");
        this.name = new String(name);
        demoList = new ArrayList(3);
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = new String(name);
        demoList = new ArrayList(3);
    }

    public void setIsPlayed(boolean is)
    {
        isPlayed = is;
    }

    public void setPlayed(boolean value)
    {
        isPlayed = value;
    }

    public boolean isPlayed()
    {
        return isPlayed;
    }

    public void addDemo(String url, String pov, String round)
    {
        demoList.add(new Demo(url, pov, round));
    }

    public void deleteDemos()
    {
        demoList = new ArrayList(3);
    }

    public Demo getDemo(int index)
    {
        return (Demo)demoList.get(index);
    }

    public int getNumDemos()
    {
        return demoList.size();
    }

    public String demoString()
    {
        return demoString("");
    }

    public String demoString(String prefix)
    {
        String ret = null;
        Demo d;
        for(int i = 0; (d = (Demo)demoList.get(i)) != null; i++)
        {
            if(ret == null)
                ret = "";
            ret = new String((new StringBuffer(ret)).append(prefix).append(d.url).append(":").append(d.pov).append(":").append(d.round).append("\n\r"));
        }

        return ret;
    }

    private static final long serialVersionUID = 0x3e8cd53cb881f7b3L;
    public static final int MAXNUMHALFS = 10;
    public static final int MAPNOTSELECTED = -1;
    public static final int TIE = -1;
    protected boolean isPlayed;
    public String scheduled;
    public String log;
    public String referee;
    public String name;
    public String screenshot;
    private ArrayList demoList;
    MatchPlacingInfo placingInfo;
    public static Data data;

}
