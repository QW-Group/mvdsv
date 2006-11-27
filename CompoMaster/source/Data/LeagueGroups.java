/**

$Id: LeagueGroups.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.io.*;
import java.util.ArrayList;
import nilzorlib.diverse.StringTools;

// Referenced classes of package Data:
//            GroupStructure, LeagueGroup, Player, DeathMatch, 
//            DMRound, Match, MatchList, Group, 
//            Data

public class LeagueGroups extends GroupStructure
{

    public boolean canBePlayed(Match m)
    {
        return true;
    }

    public LeagueGroups(Data data, boolean drawsEnabled, int pointsVictory, int pointsDraw)
    {
        super(data);
        this.drawsEnabled = false;
        this.pointsDraw = 1;
        this.pointsVictory = 2;
        pointsForMap = false;
        divisionSystem = false;
        this.drawsEnabled = drawsEnabled;
        this.pointsDraw = pointsDraw;
        this.pointsVictory = pointsVictory;
    }

    public void setDivisionSystem(boolean b)
    {
        divisionSystem = b;
    }

    public boolean isDivisionSystem()
    {
        return divisionSystem;
    }

    public int getPointsDraw()
    {
        return pointsDraw;
    }

    public void setPointsForMatch(boolean b)
    {
        pointsForMap = !b;
    }

    public boolean isPointsForMatch()
    {
        return !pointsForMap;
    }

    public int getPointsVictory()
    {
        return pointsVictory;
    }

    public boolean isDrawsEnabled()
    {
        return drawsEnabled;
    }

    public String findMatchId(Match target)
    {
        for(int i = 0; i < super.groupList.size(); i++)
        {
            LeagueGroup g = (LeagueGroup)super.groupList.get(i);
            Match m;
            for(int j = 0; (m = g.getMatch(j)) != null; j++)
                if(m == target)
                    return new String((new StringBuffer(i)).append(":").append(j));

        }

        return null;
    }

    public boolean getDrawsEnabled()
    {
        return drawsEnabled;
    }

    public Match getMatch(int matchNr)
    {
        if(matchNr < 0 || matchNr >= super.groupList.size())
            return null;
        else
            return (LeagueGroup)super.groupList.get(matchNr);
    }

    public void initCompo()
    {
        commonInit();
        generateMatches();
    }

    public void generateMatches()
    {
        LeagueGroup m;
        for(int i = 0; (m = (LeagueGroup)getMatch(i)) != null; i++)
            m.generateMatches();

    }

    public void addGroup(int size, String name)
    {
        LeagueGroup g = new LeagueGroup(this, size);
        g.setName(name);
        super.groupList.add(g);
        if(size > super.largestGroupSize)
            super.largestGroupSize = size;
    }

    public void mousePressed(MouseEvent e)
    {
        super.mousePressed(e);
        Point p = e.getPoint();
        Player pl = getPlayerAt(p.x, p.y);
        if(pl == null)
            return;
        LeagueGroup m = (LeagueGroup)getMatchAt(p);
        if(m != Group.markedGroup)
        {
            Group.markedGroup = m;
            Group.markedPlayer[0] = null;
            Group.markedPlayer[1] = null;
        }
        Player m0 = Group.markedPlayer[0];
        Player m1 = Group.markedPlayer[1];
        if(m0 == null)
            Group.markedPlayer[0] = pl;
        else
        if(m1 == pl)
            Group.markedPlayer[1] = null;
        else
        if(m0 == pl)
        {
            Group.markedPlayer[0] = m1;
            Group.markedPlayer[1] = null;
        } else
        {
            Group.markedPlayer[1] = pl;
        }
        drawCompo(new Point(0, 0));
        if(super.myContainer != null)
            super.myContainer.repaint();
        Dimension d = new Dimension(1, 2);
        Dimension cSize = super.data.getCanvasSize();
        Point drawPoint = new Point((getDrawingSize().width - d.width) / 2, p.y - d.height);
        if(drawPoint.y < 0)
            drawPoint.y = 0;
        if(drawPoint.y + d.height > cSize.height)
            drawPoint.y = cSize.height - d.height;
        DeathMatch match = m.getMarkedMatch();
        showMatchDetails(match, drawPoint);
        if(match != null)
            dispatchMatchClickEvents(match, e);
    }

    public void importMatches(BufferedReader in, Data data)
    {
        DeathMatch m = null;
        int gi = -1;
        try
        {
            do
            {
                String line;
                if((line = in.readLine()) == null)
                    break;
                if(line.startsWith("[groupdef]"))
                {
                    for(char groupName = 'A'; (line = in.readLine()) != null && !line.startsWith("[end]"); groupName++)
                    {
                        String value[];
                        if((value = StringTools.eqSplit("group", ',', line)) == null)
                            continue;
                        LeagueGroup g = new LeagueGroup(this);
                        for(int i = 0; i < value.length; i++)
                            try
                            {
                                g.addPlayer(data.getPlayer(Integer.parseInt(value[i])));
                            }
                            catch(NumberFormatException e)
                            {
                                System.out.println("NumberformatError (adding player)");
                            }

                        g.setName(GROUPIDENTIFIER + groupName);
                        super.groupList.add(g);
                    }

                }
                if(line.startsWith("[match"))
                {
                    gi = Integer.parseInt(line.substring(line.indexOf('@') + 1, line.indexOf(']')));
                    m = new DeathMatch("");
                }
                if(gi >= 0 && super.groupList.get(gi) != null)
                {
                    String value[];
                    if((value = StringTools.eqSplit("result", '-', line)) != null)
                    {
                        m.addRound(new DMRound(Integer.parseInt(value[0]), Integer.parseInt(value[1]), value[2]));
                        m.isPlayed = true;
                    }
                    if((value = StringTools.eqSplit("teams", '-', line)) != null)
                    {
                        m.setPlayer(0, data.getPlayer(Integer.parseInt(value[0])));
                        m.setPlayer(1, data.getPlayer(Integer.parseInt(value[1])));
                    }
                    if((value = StringTools.eqSplit("scheduled", '-', line)) != null)
                        m.scheduled = value[0];
                    if((value = StringTools.eqSplit("screenshot", '-', line)) != null)
                        m.screenshot = value[0];
                    if((value = StringTools.eqSplit("referee", '-', line)) != null)
                        m.referee = value[0];
                    if((value = StringTools.eqSplit("log", '-', line)) != null)
                        m.log = value[0];
                    if((value = StringTools.eqSplit("demo", '&', line)) != null)
                        m.addDemo(value[0], value[1], value[2]);
                    if(line.startsWith("[end]"))
                    {
                        m.calculateScore();
                        ((LeagueGroup)super.groupList.get(gi)).addMatch(m);
                        gi = -1;
                    }
                }
            } while(true);
        }
        catch(IOException e)
        {
            System.out.println("IO error reading match file");
        }
        super.numGroups = super.groupList.size();
    }

    private static final long serialVersionUID = 0x2824728b47454721L;
    public static final String GROUPIDENTIFIER = "Gruppe ";
    protected boolean drawsEnabled;
    protected int pointsDraw;
    protected int pointsVictory;
    protected boolean pointsForMap;
    protected boolean divisionSystem;

}