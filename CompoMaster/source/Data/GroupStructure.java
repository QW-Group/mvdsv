/**

$Id: GroupStructure.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.*;
import java.util.ArrayList;

// Referenced classes of package Data:
//            MatchList, Group, LeagueGroups, DeathMatch, 
//            Match, Data, Player

public abstract class GroupStructure extends MatchList
{

    public abstract Match getMatch(int i);

    public abstract void initCompo();

    public abstract void addGroup(int i, String s);

    public GroupStructure(Data data)
    {
        columns = 2;
        largestGroupSize = 0;
        groupList = new ArrayList(10);
        super.data = data;
    }

    public void reset()
    {
        super.reset();
        groupList = new ArrayList(10);
    }

    public int getNumGroups()
    {
        return numGroups;
    }

    public int getNumMatches()
    {
        return numGroups;
    }

    public void setNumGroups(int x)
    {
        numGroups = x;
    }

    public int getLargestGroupSize()
    {
        if(largestGroupSize != 0)
            return largestGroupSize;
        else
            return findLargestGroupSize();
    }

    private int findLargestGroupSize()
    {
        for(int i = 0; i < groupList.size(); i++)
        {
            Group g = (Group)groupList.get(i);
            int gs = g.getNumPlayers();
            if(gs > largestGroupSize)
                largestGroupSize = gs;
        }

        return largestGroupSize;
    }

    public void commonInit()
    {
        int b = super.data.getNumPlayers() % numGroups;
        int a = 0;
        for(int i = 0; i < numGroups; i++)
        {
            a += b;
            int extra;
            if(a >= numGroups)
            {
                a -= numGroups;
                extra = 1;
            } else
            {
                extra = 0;
            }
            addGroup(super.data.getNumPlayers() / numGroups + extra, "Group ".concat(String.valueOf(i + 1)));
        }

        if(super.data.doShuffle)
            super.data.shufflePlayerList();
        if((this instanceof LeagueGroups) && ((LeagueGroups)this).isDivisionSystem())
        {
            int i = 0;
            int gPtr = 0;
            for(; i < super.data.getNumPlayers(); i++)
                if(!((Group)getMatch(gPtr)).addPlayer(super.data.getPlayer(i)))
                {
                    ((Group)getMatch(gPtr)).setName(new String((new StringBuffer("Division ")).append(gPtr + 1)));
                    gPtr++;
                    i--;
                }

        } else
        {
            int inc = 1;
            int i = 0;
            int gPtr = 0;
            for(; i < super.data.getNumPlayers(); i++)
            {
                if(!((Group)getMatch(gPtr)).addPlayer(super.data.getPlayer(i)))
                {
                    gPtr += inc;
                    i--;
                    continue;
                }
                gPtr += inc;
                if(gPtr == numGroups)
                {
                    gPtr--;
                    inc = -1;
                }
                if(gPtr == -1)
                {
                    gPtr = 0;
                    inc = 1;
                }
            }

        }
    }

    public Dimension getDrawingSize()
    {
        if(groupList.size() == 0)
            return new Dimension(0, 0);
        int eachHeight = getEachGroupSize().height;
        int totalHeight = 20;
        totalHeight += eachHeight * (numGroups / columns);
        if(numGroups % columns != 0)
            totalHeight += eachHeight;
        totalHeight -= 40;
        totalHeight += 15;
        return new Dimension(20 + ((Group)getMatch(0)).getGroupWidth() * columns + 40 * (columns - 1), totalHeight);
    }

    public void setColumns(int i)
    {
        columns = i;
    }

    public void drawCompo(Point offset)
    {
        Graphics g = super.compoGraphics;
        if(groupList.size() == 0)
            return;
        Dimension dSize = super.data.getCanvasSize();
        g.setFont(MatchList.getMainFont());
        g.setColor(super.bgcolor);
        g.fillRect(offset.x, offset.y, dSize.width, dSize.height);
        Point drawPoint = null;
        DeathMatch markedCheck = null;
        int markedIndex = -1;
        for(int i = 0; i < numGroups; i++)
        {
            Point p = getMatchPos(i);
            Match m = getMatch(i);
            m.drawMatch(p.x, p.y, g);
        }

        drawVersionInfo(g);
    }

    protected Point getMatchPos(int idx)
    {
        if(idx < 0 || idx >= groupList.size())
        {
            return null;
        } else
        {
            Dimension each = getEachGroupSize();
            Point p = new Point(10, 10);
            p.x += (idx % columns) * each.width;
            p.y += (idx / columns) * each.height;
            return p;
        }
    }

    public Match getMatchAt(Point q)
    {
        if(getMatch(0) == null)
        {
            return null;
        } else
        {
            Dimension each = getEachGroupSize();
            Point p = new Point((q.x - 10) / each.width, (q.y - 10) / each.height);
            return getMatch(p.y * columns + p.x);
        }
    }

    public Player getPlayerAt(int x, int y)
    {
        Dimension each = getEachGroupSize();
        Point p = new Point((x - 10) / each.width, (y - 10) / each.height);
        Match m = getMatch(p.y * columns + p.x);
        if(m == null)
        {
            return null;
        } else
        {
            Point q = new Point(x - p.x * each.width, y - p.y * each.height);
            int i = (q.y - 10 - 44) / 20;
            return m.getPlayer(i);
        }
    }

    public Dimension getEachGroupSize()
    {
        if(eachGroupSize != null)
            return eachGroupSize;
        int maxP = getMatch(0).getNumPlayers();
        int i = 0;
        do
        {
            Group g;
            if((g = (Group)getMatch(i)) == null)
                break;
            int thisP = g.getNumPlayers();
            if(thisP > maxP)
            {
                maxP = thisP;
                break;
            }
            i++;
        } while(true);
        int eachHeight = maxP * 20 + 44 + 40;
        int eachWidth = ((Group)getMatch(0)).getGroupWidth() + 40;
        eachGroupSize = new Dimension(eachWidth, eachHeight);
        return eachGroupSize;
    }

    public int getNumAdvancingPlayers()
    {
        return numAdvancingPlayers;
    }

    private static final long serialVersionUID = 0x599e750a69fc3514L;
    public static final int GA = 40;
    protected int columns;
    public int numAdvancingPlayers;
    public boolean advancementEnabled;
    protected ArrayList groupList;
    protected int numGroups;
    protected int largestGroupSize;
    private transient Dimension eachGroupSize;

}