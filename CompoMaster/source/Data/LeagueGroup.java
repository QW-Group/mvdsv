/**

$Id: LeagueGroup.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.*;
import java.io.PrintStream;
import java.util.ArrayList;

// Referenced classes of package Data:
//            Group, DeathMatch, Player, RealPlayer, 
//            Match, MatchList, Data, GroupStructure, 
//            LeagueGroups, Comparable

public class LeagueGroup extends Group
{
    private class MatchPlayerInfo
        implements Comparable
    {

        public String getColumnData(int index)
        {
            switch(filterIndex(index))
            {
            case 0: // '\0'
                return player.getName();

            case 1: // '\001'
                return String.valueOf(numPlayed);

            case 2: // '\002'
                return String.valueOf(won);

            case 3: // '\003'
                return String.valueOf(draw);

            case 4: // '\004'
                return String.valueOf(lost);

            case 5: // '\005'
                return String.valueOf(mapsWon);

            case 6: // '\006'
                return String.valueOf(mapsLost);

            case 7: // '\007'
                return String.valueOf(kills);

            case 8: // '\b'
                return String.valueOf(deaths);

            case 9: // '\t'
                return String.valueOf(diff);

            case 10: // '\n'
                return String.valueOf(points);
            }
            return "Error";
        }

        public int compareTo(Object b)
        {
            MatchPlayerInfo mb = (MatchPlayerInfo)b;
            if(disabled && mb.disabled)
                return 1;
            if(disabled)
                return -1;
            if(mb.disabled)
                return 1;
            if(points == mb.points)
            {
                int whoWon = 0;
                if(whoWon == 0)
                {
                    if(mapsWon - mapsLost == mb.mapsWon - mb.mapsLost)
                    {
                        if(diff == mb.diff)
                            return 0;
                        if(diff > mb.diff)
                            return 1;
                    }
                    if(mapsWon - mapsLost > mb.mapsWon - mb.mapsLost)
                        return 1;
                } else
                if(whoWon < 0)
                    return 1;
            } else
            if(points > mb.points)
                return 1;
            return -1;
        }

        private int numPlayed;
        private int won;
        private int lost;
        private int draw;
        private int mapsWon;
        private int mapsLost;
        private int kills;
        private int deaths;
        private int diff;
        private int points;
        private boolean disabled;
        private Player player;










        MatchPlayerInfo(int idx)
        {
            won = 0;
            lost = 0;
            draw = 0;
            mapsWon = 0;
            mapsLost = 0;
            kills = 0;
            deaths = 0;
            diff = 0;
            points = 0;
            Player thisOne = (Player)playerList.get(idx);
            player = thisOne;
            disabled = thisOne.isDisabled();
            boolean isPFM = isPointsForMatch();
            for(int i = 0; i < matchList.size(); i++)
            {
                DeathMatch m = (DeathMatch)matchList.get(i);
                if(m.isDisabled() || !m.isPlayed() || m.getPlayer(0) != thisOne && m.getPlayer(1) != thisOne)
                    continue;
                numPlayed++;
                if(m.getWinner() == thisOne)
                {
                    won++;
                    if(isPFM)
                        points += getPointsVictory();
                } else
                if(m.isDraw())
                {
                    draw++;
                    if(isPFM)
                        points += getPointsDraw();
                } else
                {
                    lost++;
                }
                int a;
                int b;
                if(m.getPlayer(0) == thisOne)
                {
                    a = 0;
                    b = 1;
                } else
                {
                    a = 1;
                    b = 0;
                }
                mapsWon += m.getScore(a);
                mapsLost += m.getScore(b);
                if(!isPFM)
                    points += m.getScore(a);
                int numRounds = m.getNumRounds();
                for(int j = 0; j < numRounds; j++)
                {
                    kills += m.getFrags(j, a);
                    deaths += m.getFrags(j, b);
                }

                diff = kills - deaths;
            }

        }
    }


    public LeagueGroup(LeagueGroups l)
    {
        isSorted = false;
        isInitialized = false;
        leagueGroups = l;
        init(true);
        maxSize = 1000;
    }

    public LeagueGroup(LeagueGroups l, int size)
    {
        isSorted = false;
        isInitialized = false;
        leagueGroups = l;
        init(true);
        maxSize = size;
    }

    public DeathMatch getMatch(int idx)
    {
        if(idx < 0 || idx >= matchList.size())
            return null;
        else
            return (DeathMatch)matchList.get(idx);
    }

    public DeathMatch getMarkedMatch()
    {
        if(Group.markedPlayer[0] != null && Group.markedPlayer[1] != null)
        {
            for(int i = 0; i < matchList.size(); i++)
            {
                DeathMatch m = (DeathMatch)matchList.get(i);
                Player p0 = m.getPlayer(0);
                Player p1 = m.getPlayer(1);
                if(p0 == Group.markedPlayer[0] && p1 == Group.markedPlayer[1] || p0 == Group.markedPlayer[1] && p1 == Group.markedPlayer[0])
                    return m;
            }

        }
        return null;
    }

    private void init(boolean first)
    {
        isInitialized = true;
        tableWidth = 0;
        int len = getColumnCount();
        for(int i = 0; i < len; i++)
            tableWidth += getColumnWidth(i);

        if(first)
        {
            playerList = new ArrayList(10);
            matchList = new ArrayList(10);
        }
    }

    public int getGroupWidth()
    {
        if(!isInitialized)
            init(false);
        return tableWidth;
    }

    public void generateMatches()
    {
        matchList = new ArrayList(5);
        int k = 0;
        for(int i = 0; i < playerList.size(); i++)
        {
            for(int j = i + 1; j < playerList.size(); j++)
                matchList.add(new DeathMatch((Player)playerList.get(i), (Player)playerList.get(j), String.valueOf(++k)));

        }

    }

    public boolean isPlayed()
    {
        if(super.isPlayed)
            return true;
        for(int i = 0; i < matchList.size(); i++)
        {
            DeathMatch m = (DeathMatch)matchList.get(i);
            if(!((Match) (m)).isPlayed && !m.isDisabled())
                return false;
        }

        return true;
    }

    public boolean addPlayer(Player p)
    {
        if(playerList.size() == maxSize)
        {
            return false;
        } else
        {
            playerList.add(p);
            return true;
        }
    }

    public void addMatch(DeathMatch m)
    {
        m.name = String.valueOf(matchList.size());
        matchList.add(m);
    }

    public int getNumPlayers()
    {
        return playerList.size();
    }

    public Player getPlayer(int i)
    {
        if(i < 0 || i >= playerList.size())
            return null;
        else
            return (Player)playerList.get(i);
    }

    public void setPlayer(int i, Player p)
    {
        playerList.set(i, p);
    }

    protected void drawMatchSub(int x, int y, Graphics g)
    {
        if(tableWidth == 0)
            init(false);
        int origY = y;
        FontMetrics fm = g.getFontMetrics(MatchList.getMainFont());
        g.setColor(Group.headerCol);
        g.drawString("Matches played: ", x + 5 + getColumnWidth(0), (y + 20) - 5);
        int numPlayed = 0;
        int total = 0;
        for(int i = 0; i < matchList.size(); i++)
        {
            DeathMatch dm = (DeathMatch)matchList.get(i);
            if(dm.isDisabled())
                continue;
            total++;
            if(dm.isPlayed())
                numPlayed++;
        }

        if(isPlayed())
            g.setColor(Group.greenTextCol);
        g.drawString(new String((new StringBuffer("                ")).append(numPlayed).append(" of ").append(total)), x + 5 + getColumnWidth(0), (y + 20) - 5);
        g.drawString(getName(), x + 5, (y + 20) - 5);
        y += 35;
        y += 4;
        int xoffset = x + 5;
        int colCount = getColumnCount();
        int groupHeight = getNumPlayers() * 20 + 44;
        sortPlayers();
        for(int j = 0; j < getNumPlayers(); j++)
        {
            if(j % 2 == 0)
            {
                g.setColor(Group.BG1);
                g.fillRect(x + 2, y + 5, getGroupWidth() - 2, 20);
            }
            if(getPlayer(j).isDisabled())
            {
                g.setColor(Group.disabledCol);
                g.fillRect(x + 2, y + 5, getGroupWidth() - 2, 20);
            }
            if(this == Group.markedGroup && (getPlayer(j) == Group.markedPlayer[0] || getPlayer(j) == Group.markedPlayer[1]))
            {
                g.setColor(Group.highlightCol);
                g.fillRect(x + 2, y + 5, getGroupWidth() - 2, 20);
            }
            y += 20;
            g.setColor(Group.textCol);
            String name = ((Player)playerList.get(j)).getName();
            Data.drawName(g, name, x + 5, y - 2);
            MatchPlayerInfo mpi = new MatchPlayerInfo(j);
            g.setColor(Group.textCol);
            xoffset = x + 5 + getColumnWidth(0);
            for(int i = 1; i < colCount; i++)
            {
                g.setColor(getColumnColor(i));
                g.drawString(mpi.getColumnData(i), xoffset, y - 2);
                xoffset += getColumnWidth(i);
            }

        }

        y = (origY + 20) - 5;
        xoffset = x + 5;
        for(int i = 0; i < colCount; i++)
        {
            g.setColor(Group.headerCol);
            g.drawString(getColumnHeader(i), xoffset, y + 20);
            xoffset += getColumnWidth(i);
            g.setColor(Color.white);
            g.drawLine(xoffset - 4, y + 4, xoffset - 4, (y + groupHeight + 4) - 20);
        }

        xoffset = x + 5;
        g.drawLine((xoffset - 4) + getColumnWidth(0), y + 4, (xoffset - 4) + getGroupWidth(), y + 4);
        int greenPos = 0;
        if(leagueGroups != null)
            greenPos = leagueGroups.getNumAdvancingPlayers();
        if(greenPos > 0)
        {
            y = origY + greenPos * 20;
            g.setColor(Group.advanceCol);
            g.drawLine(x, y + 3 + 40, x + getGroupWidth(), 3 + y + 40);
        }
    }

    public Object getPlayerAt(int rank)
    {
        if(rank < 1 || rank > playerList.size())
            return null;
        if(!isSorted)
        {
            sortPlayers();
            isSorted = true;
        }
        return (RealPlayer)playerList.get(rank - 1);
    }

    private void sortPlayers()
    {
        if(Data.debug)
            System.out.println(new String((new StringBuffer("    SORT [")).append(getName()).append("]")));
        MatchPlayerInfo mpi[] = new MatchPlayerInfo[playerList.size()];
        int numPlayers = playerList.size();
        for(int i = 0; i < numPlayers; i++)
            mpi[i] = new MatchPlayerInfo(i);

        for(int i = 0; i < numPlayers - 1; i++)
        {
            int greatest = i;
            for(int j = i + 1; j < numPlayers; j++)
                if(mpi[greatest].compareTo(mpi[j]) < 0)
                    greatest = j;

            if(greatest != i)
            {
                Player temp = (Player)playerList.get(i);
                playerList.set(i, playerList.get(greatest));
                playerList.set(greatest, temp);
                MatchPlayerInfo t = mpi[i];
                mpi[i] = mpi[greatest];
                mpi[greatest] = t;
            }
        }

    }

    private int whoWon(Player a, Player b)
    {
        int size = matchList.size();
        for(int i = 0; i < size; i++)
        {
            DeathMatch m = (DeathMatch)matchList.get(i);
            if(m.getPlayer(0) == a && m.getPlayer(1) == b)
            {
                if(m.getWinner() == null)
                    return 0;
                return m.getWinner() != a ? 1 : -1;
            }
            if(m.getPlayer(0) == b && m.getPlayer(1) == a)
            {
                if(m.getWinner() == null)
                    return 0;
                return m.getWinner() != b ? 1 : -1;
            }
        }

        return 0;
    }

    public void deleteDisabledPlayers()
    {
        RealPlayer p;
        for(int i = 0; (p = (RealPlayer)playerList.get(i)) != null; i++)
            if(p.disabled)
            {
                playerList.remove(i);
                i--;
            }

    }

    public void autoSchedule(int maxPass)
    {
        int numMat = matchList.size();
        int numRnds = playerList.size() - 1;
        if(playerList.size() % 2 == 1)
            numRnds++;
        int pass = 0;
        boolean success;
        do
        {
            ArrayList open = new ArrayList(numMat);
            ArrayList closed = new ArrayList(numMat);
            DeathMatch round[][] = new DeathMatch[numRnds][playerList.size() / 2];
            int mptr = 0;
            int btc = 1;
            int sbtc = 2;
            boolean redo = false;
            for(int i = 0; i < numMat; i++)
                open.add(matchList.get(i));

            pass++;
            success = true;
            System.out.println("-Pass #".concat(String.valueOf(pass)));
            for(int i = 0; i < numRnds; i++)
            {
                System.out.println(new String((new StringBuffer("--Round ")).append(i).append(": ---")));
                mptr = 0;
                btc = 1;
                for(int j = 0; j < playerList.size() / 2; j++)
                {
                    DeathMatch mIn;
                    do
                    {
                        redo = false;
                        mIn = (DeathMatch)open.get(mptr);
                        if(mIn == null)
                        {
                            int bt = calcBackTrace(btc++);
                            if(j - bt < 0)
                            {
                                System.out.println(new String((new StringBuffer("-Pass #")).append(pass).append(" result: failure.\n")));
                                success = false;
                                j = playerList.size();
                                i = numRnds;
                                break;
                            }
                            mptr = 0;
                            j--;
                            for(int l = 0; l < bt; l++)
                            {
                                round[i][j].scheduled = "N/A";
                                open.add(round[i][j--]);
                            }

                            redo = false;
                        } else
                        {
                            for(int k = 0; k < j; k++)
                                if(mIn.getPlayer(0) == round[i][k].getPlayer(0) || mIn.getPlayer(0) == round[i][k].getPlayer(1) || mIn.getPlayer(1) == round[i][k].getPlayer(0) || mIn.getPlayer(1) == round[i][k].getPlayer(1))
                                {
                                    mptr++;
                                    redo = true;
                                }

                        }
                    } while(redo);
                    if(mIn != null)
                    {
                        round[i][j] = mIn;
                        open.remove(mptr);
                        round[i][j].scheduled = new String((new StringBuffer("")).append(i + 1));
                    }
                }

            }

            if(pass == maxPass && !success)
            {
                System.out.println(new String((new StringBuffer("Unable to compute after ")).append(maxPass).append(" attempts. This algorithm is based on randomness, and MAY ").append("succeed if retried. Reexecute the command if you feel lucky. ")));
                success = true;
            } else
            if(success)
                System.out.println(new String((new StringBuffer("-Pass #")).append(pass).append(" result: Success!")));
        } while(!success);
    }

    private int calcBackTrace(int btc)
    {
        if(btc < 1)
            return 1;
        int out = btc ^ btc - 1;
        out &= btc;
        int i;
        for(i = 1; (out >>= 1) != 0; i++);
        return i;
    }

    private boolean isPointsForMatch()
    {
        if(leagueGroups == null)
            return true;
        else
            return leagueGroups.isPointsForMatch();
    }

    private int getPointsDraw()
    {
        if(leagueGroups == null)
            return defaultPointsDraw;
        else
            return leagueGroups.getPointsDraw();
    }

    private int getPointsVictory()
    {
        if(leagueGroups == null)
            return defaultPointsVictory;
        else
            return leagueGroups.getPointsVictory();
    }

    private boolean isDrawsEnabled()
    {
        if(leagueGroups == null)
            return defaultDrawsEnabled;
        else
            return leagueGroups.isDrawsEnabled();
    }

    protected int filterIndex(int index)
    {
        if(!isDrawsEnabled() && index >= 3)
            return index + 1;
        else
            return index;
    }

    private int getColumnCount()
    {
        if(isDrawsEnabled())
            return colWidth.length;
        else
            return colWidth.length - 1;
    }

    private String getColumnHeader(int index)
    {
        index = filterIndex(index);
        return colName[index];
    }

    private int getColumnWidth(int index)
    {
        index = filterIndex(index);
        return colWidth[index];
    }

    private Color getColumnColor(int index)
    {
        if(filterIndex(index) == 10)
            return Color.yellow;
        else
            return Color.white;
    }

    public String toString()
    {
        return new String((new StringBuffer(String.valueOf(String.valueOf(super.name)))).append(": ").append(playerList.size()).append(" players."));
    }

    public String stringStandings()
    {
        String ret = "";
        sortPlayers();
        for(int j = 0; j < getNumPlayers(); j++)
            if(!getPlayer(j).isDisabled() || Data.showDisabled)
            {
                MatchPlayerInfo mpi = new MatchPlayerInfo(j);
                ret = new String((new StringBuffer(ret)).append(((Player)playerList.get(j)).getName()).append(":").append(mpi.numPlayed).append(":").append(mpi.won).append(":").append(mpi.lost).append(":").append(mpi.mapsWon).append(":").append(mpi.mapsLost).append(":").append(mpi.kills).append(":").append(mpi.deaths).append(":").append(mpi.diff).append(":").append(mpi.points).append("\n\r"));
            }

        return ret;
    }

    public String stringMatches()
    {
        String ret = "";
        int index = 0;
        for(int i = 0; i < matchList.size(); i++)
        {
            DeathMatch m = (DeathMatch)matchList.get(i);
            if(!m.isDisabled() || Data.showDisabled)
                ret = new String((new StringBuffer(ret)).append(index++).append(":").append(m.toString()).append("\r\n"));
        }

        return ret;
    }

    private static final long serialVersionUID = 0x8ca1e5e84a9d92daL;
    private static int tableWidth;
    private static int defaultPointsDraw = 1;
    private static int defaultPointsVictory = 2;
    private static boolean defaultDrawsEnabled = false;
    private transient boolean isSorted;
    private transient boolean isInitialized;
    private static String colName[] = {
        "Name", "Mp", "W", "D", "L", "Wm", "Lm", "Fr", "Dt", "Dif", 
        "Po"
    };
    private static int colWidth[] = {
        130, 20, 20, 20, 20, 20, 20, 40, 40, 40, 
        20
    };
    private int maxSize;
    private LeagueGroups leagueGroups;
    public int xPos;
    public int yPos;
    ArrayList playerList;
    ArrayList matchList;




}