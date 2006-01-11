// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:15:52
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Data.java

package Data;

import java.awt.Dimension;
import java.awt.Graphics;
import java.io.*;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import nilzorlib.diverse.StringTools;

// Referenced classes of package Data:
//            Player, Team, RealPlayer, FDECup, 
//            FFAGroups, LeagueGroups, RankOf, Group, 
//            GroupStructure, Walkover, MatchList, ProgressIndicator

public class Data
    implements Serializable
{

    public Data()
    {
        isChanged = false;
        doMaps = false;
        doRounds = false;
        isLaunched = false;
        enableQ3Colors = true;
        doShuffle = true;
        cupSliderVal = 0;
        player = new ArrayList(16);
        mapName = new ArrayList(8);
        roundNameShort = new String[2];
        roundNameLong = new String[2];
        teamMode = false;
    }

    public void setInDataFilename(String fname)
    {
        inDataFilename = fname;
    }

    public String getInDataFilename()
    {
        return inDataFilename;
    }

    public void setOutDataFilename(String fname)
    {
        outDataFilename = fname;
    }

    public String getOutDataFilename()
    {
        return outDataFilename;
    }

    public void setRoundName(int idx, String shortName, String longName)
    {
        roundNameShort[idx] = shortName;
        roundNameLong[idx] = longName;
    }

    public String getRoundNameShort(int idx)
    {
        return roundNameShort[idx];
    }

    public String getRoundNameLong(int idx)
    {
        return roundNameLong[idx];
    }

    public int getNumPlayers()
    {
        return player.size();
    }

    public void setCompoName(String s)
    {
        compoName = s;
    }

    public String getCompoName()
    {
        return compoName;
    }

    public void setCanvasSize(Dimension d)
    {
        canvasSize = d;
    }

    public Dimension getCanvasSize()
    {
        if(canvasSize != null)
            return canvasSize;
        else
            return matchList.getDrawingSize();
    }

    public void setDoShuffle(boolean val)
    {
        doShuffle = val;
    }

    public void addPlayer(Player p)
    {
        player.add(p);
    }

    public void teamsToPlayers()
    {
        int sz = player.size();
        for(int i = 0; i < sz; i++)
        {
            Player p = (Player)player.get(i);
            if(p instanceof Team)
            {
                Player q = new RealPlayer(p.getName(), ((Team)p).getPassword(), p.getRank());
                player.set(i, q);
            }
        }

    }

    public void playersToTeams()
    {
        int sz = player.size();
        for(int i = 0; i < sz; i++)
        {
            Player p = (Player)player.get(i);
            if(p instanceof RealPlayer)
            {
                Player q = new Team(p.getName(), ((RealPlayer)p).getPassword(), p.getRank(), null, null, null);
                player.set(i, q);
            }
        }

    }

    public void removeWalkoverPlayers()
    {
        int len = player.size();
        for(int i = 0; i < len; i++)
            if(!(player.get(i) instanceof RealPlayer))
            {
                player.remove(i--);
                len--;
            }

    }

    public Player getPlayer(int idx)
    {
        return (Player)player.get(idx);
    }

    public boolean removePlayer(int idx)
    {
        try
        {
            player.remove(idx);
            boolean flag = true;
            return flag;
        }
        catch(IndexOutOfBoundsException e)
        {
            boolean flag1 = false;
            return flag1;
        }
    }

    public void addMap(String s)
    {
        mapName.add(s);
        isChanged = true;
    }

    public void removeMap(int idx)
    {
        mapName.remove(idx);
    }

    public String getMapName(int idx)
    {
        if(mapName.size() < 1)
            return null;
        if(idx >= mapName.size())
            return null;
        else
            return (String)mapName.get(idx);
    }

    public ArrayList getMapList()
    {
        return mapName;
    }

    public int getMapIndex(String map)
    {
        String s;
        for(int i = 0; (s = getMapName(i)) != null; i++)
            if(s.equals(map))
                return i;

        return -1;
    }

    public ArrayList getPlayerList()
    {
        return player;
    }

    public void sortPlayers(int criteria)
    {
        if(criteria == 0)
            shufflePlayerList(true);
    }

    public int getNumMaps()
    {
        return mapName.size();
    }

    public void setModeFDE(int dmWinMode, int numFinales, int numSERounds)
    {
        matchList = new FDECup(this, dmWinMode, numFinales, numSERounds);
    }

    public void setModeFFA()
    {
        matchList = new FFAGroups(this);
    }

    public void setModeLeague(boolean enableDraws, int pointsVictory, int pointsDraw)
    {
        matchList = new LeagueGroups(this, enableDraws, pointsVictory, pointsDraw);
    }

    public boolean getTeamMode()
    {
        return teamMode;
    }

    public void setTeamMode(boolean teamMode)
    {
        this.teamMode = teamMode;
    }

    public void clearPlayerList()
    {
        player = new ArrayList(16);
    }

    public void clearMapList()
    {
        mapName = new ArrayList(4);
    }

    public void shufflePlayerList()
    {
        shufflePlayerList(false);
    }

    public void shufflePlayerList(boolean onlySort)
    {
        int numPlayers = player.size();
        Player tempPlayer[] = new Player[numPlayers];
        for(int i = 0; i < numPlayers; i++)
        {
            int rnd;
            if(onlySort)
                rnd = i;
            else
                rnd = (int)(Math.random() * (double)numPlayers);
            if(tempPlayer[rnd] != null)
                i--;
            else
                tempPlayer[rnd] = (Player)player.get(i);
        }

        player = new ArrayList(numPlayers);
        for(int rk = 1; rk <= MAXRANK; rk++)
        {
            for(int i = 0; i < numPlayers; i++)
                if(tempPlayer[i].getRank() == rk)
                    player.add(tempPlayer[i]);

        }

    }

    public static Data load(URL u)
        throws Exception, IOException
    {
        InputStream source = u.openStream();
        Data main = loadObjectFile(source, null);
        return main;
    }

    public static Data load(URL u, ProgressIndicator p)
        throws Exception
    {
        errors = " Error=1";
        InputStream source = u.openStream();
        errors = " Error=2";
        URLConnection temp = u.openConnection();
        int len = temp.getContentLength();
        errors = " Error=3";
        Data main = null;
        if(len < 1)
        {
            errors = " Error=3.1";
            main = loadObjectFile(source, null);
        } else
        {
            errors = " Error=3.2";
            if(source == null)
                errors = "File not found";
            if(p != null)
                p.setGoal(len);
            main = loadObjectFile(source, p);
        }
        return main;
    }

    public static Data load(String filename)
    {
        try
        {
            File f = new File(filename);
            if(!f.exists())
                f = new File(String.valueOf(String.valueOf(filename)).concat(".cmp"));
            FileInputStream source = new FileInputStream(f);
            Data data2 = loadObjectFile(source, null);
            return data2;
        }
        catch(FileNotFoundException e)
        {
            System.out.println("File not found error");
            Data data = null;
            return data;
        }
        catch(Exception e)
        {
            System.out.println("File not a CompoMaster file. \r\n".concat(String.valueOf(String.valueOf(e.getMessage()))));
        }
        Data data1 = null;
        return data1;
    }

    public static Data loadObjectFile(InputStream in, ProgressIndicator p)
        throws Exception
    {
        int i = 0;
        String errParam = "";
        errors = " Error=4";
        ObjectInputStream input;
        if(p != null)
        {
            byte buffer[] = new byte[p.getGoal()];
            byte temp[] = new byte[1024];
            errors = " Error=5 -Total size:".concat(String.valueOf(String.valueOf(p.getGoal())));
            int chunksize = 0;
            int pos;
            for(pos = 0; pos < p.getGoal(); pos += chunksize)
            {
                chunksize = in.read(temp, 0, 1024);
                errors = String.valueOf(String.valueOf((new StringBuffer(" Error 5.5: Chk[")).append(i++).append("]=").append(chunksize).append(" Pos=").append(pos)));
                System.arraycopy(temp, 0, buffer, pos, chunksize);
                p.setProgress(pos);
            }

            p.setProgress(p.getGoal());
            errParam = String.valueOf(String.valueOf((new StringBuffer("Pos: ")).append(pos).append(" Last chunk: ").append(chunksize).append(" BufLen: ").append(buffer.length)));
            errors = " Error=6 ".concat(String.valueOf(String.valueOf(errParam)));
            input = new ObjectInputStream(new ByteArrayInputStream(buffer));
        } else
        {
            errors = " Error=7";
            input = new ObjectInputStream(in);
        }
        errors = " Error=8 ".concat(String.valueOf(String.valueOf(errParam)));
        Object o = input.readObject();
        Data d;
        try
        {
            errors = " Error=9 ".concat(String.valueOf(String.valueOf(errParam)));
            String version = (String)o;
            d = (Data)input.readObject();
        }
        catch(ClassCastException e)
        {
            d = (Data)o;
        }
        errors = " Error=10 ".concat(String.valueOf(String.valueOf(errParam)));
        input.close();
        return d;
    }

    public static boolean save(String filename, Data d)
    {
        try
        {
            int slash1 = filename.lastIndexOf('\\');
            int slash2 = filename.lastIndexOf('/');
            int dot = filename.lastIndexOf('.');
            if(dot == -1 || dot < slash1 || dot < slash2)
                filename = String.valueOf(String.valueOf(filename)).concat(".cmp");
            FileOutputStream destination = new FileOutputStream(filename);
            ObjectOutputStream output = new ObjectOutputStream(destination);
            output.writeObject(VERSION_NO);
            output.writeObject(d);
            output.flush();
            destination.close();
        }
        catch(Exception e)
        {
            boolean flag = false;
            return flag;
        }
        return true;
    }

    public static int mirrorInt(int nr, int mirror)
    {
        int set = mirror;
        int out = 0;
        for(int check = 1; check <= mirror;)
        {
            if((nr & check) != 0)
                out |= set;
            check <<= 1;
            set >>= 1;
        }

        return out;
    }

    public void importMapList(BufferedReader in)
        throws IOException
    {
        mapName = new ArrayList();
        do
        {
            String line;
            if((line = in.readLine()) == null)
                break;
            line = line.trim();
            if(line.length() > 0)
                mapName.add(line.trim());
        } while(true);
    }

    public void exportMapList(FileWriter out)
        throws IOException
    {
        int size = mapName.size();
        String newline = System.getProperty("line.separator");
        for(int i = 0; i < size; i++)
        {
            String name = (String)mapName.get(i);
            out.write(String.valueOf(name) + String.valueOf(newline));
        }

        out.close();
    }

    public void importPlayerList(BufferedReader teamsIn)
    {
        Data d = RealPlayer.importPlayerList(teamsIn);
        if(d != null)
            player = d.getPlayerList();
    }

    public static void drawName(Graphics g, String name, int x, int y, int maxLetters)
    {
        if(maxLetters == -1)
            maxLetters = 0x7fffffff;
        StringTools.setMaxLetters(maxLetters);
        drawName(g, name, x, y);
        StringTools.setMaxLetters(MAXLETTERS);
    }

    public static void drawName(Graphics g, String name, int x, int y)
    {
        boolean colors = getCurrentInstance().enableQ3Colors;
        if(colors)
            StringTools.drawQ3String(g, name, x, y);
        else
        if(name.length() > MAXLETTERS)
            g.drawString(String.valueOf(String.valueOf(name.substring(0, MAXLETTERS - 1))).concat(".."), x, y);
        else
            g.drawString(name, x, y);
    }

    public static Data getCurrentInstance()
    {
        return currentInstance;
    }

    public static void setCurrentInstance(Data d)
    {
        currentInstance = d;
    }

    public Data getExtData()
    {
        return extData;
    }

    public void setExtData(Data edata)
    {
        extData = edata;
        if(edata == null)
            return;
        for(int i = 0; i < getNumPlayers(); i++)
        {
            Player p = getPlayer(i);
            if(!(p instanceof RankOf))
                continue;
            RankOf r = (RankOf)p;
            if(edata.matchList instanceof FDECup)
            {
                r.setSource((FDECup)edata.matchList);
            } else
            {
                int idx = r.getGroupIndex();
                r.setSource((Group)edata.matchList.getMatch(idx));
            }
        }

    }

    public boolean transferExternalPlayers()
    {
        doShuffle = false;
        clearPlayerList();
        if(extData.matchList instanceof GroupStructure)
        {
            int cnt = extData.matchList.getNumMatches();
            for(int i = 0; i < cnt; i++)
            {
                Group g = (Group)extData.matchList.getMatch(i);
                for(int j = inRankHigh; j <= inRankLow; j++)
                    if(g.getPlayerAt(j) == null)
                    {
                        addPlayer(new Walkover(j));
                    } else
                    {
                        RankOf r = new RankOf(g, j);
                        r.setGroupIndex(i);
                        addPlayer(r);
                    }

            }

        } else
        {
            for(int j = inRankHigh; j <= inRankLow; j++)
                addPlayer(new RankOf((FDECup)extData.matchList, j));

        }
        return true;
    }

    public static final int SORT_BY_RANK = 0;
    public static final int SORT_BY_NAME = 1;
    private static final long serialVersionUID = 0xd0bc90a4abf0dd8fL;
    public static final int MAXRANK = 257;
    public static int MAXLETTERS = 15;
    public static final String DATA_EXTENSION = ".cmp";
    public static final int MAXROUNDS = 10;
    public static final int STANDARD = 0;
    public static final int FDE = 1;
    public static final int HYBRID = 2;
    public static final int FFA = 3;
    public static final int LEAGUE = 4;
    public static final int COUNTMAPS = 0;
    public static final int COUNTFRAGS = 1;
    public static final boolean drawSignature = true;
    public static final String VERSION_NO = "2.9.99.01";
    public static final String VERSION = new String("Compo Master " + VERSION_NO + ", Frode Nilsen, 2003 + VVD patches, 2006");
    public static final String VERSION_2 = new String("http://www.compomaster.com.");
    public static final boolean ANONYMOUS = true;
    public static final int VERSIONTEXT_HEIGHT = 15;
    public static transient boolean debug = false;
    public static transient boolean showDisabled = true;
    public transient boolean isChanged;
    private static Data currentInstance;
    public int inRankHigh;
    public int inRankLow;
    public int outRankHigh;
    public int outRankLow;
    private String inDataFilename;
    private String outDataFilename;
    protected transient Data extData;
    public static transient String errors = "";
    private transient Dimension canvasSize;
    public static boolean doDetailsButton = true;
    public MatchList matchList;
    public boolean playerReporting;
    public boolean doMaps;
    public boolean doRounds;
    public boolean isLaunched;
    public String defaultFileName;
    public String adminPassword;
    public boolean enableQ3Colors;
    public boolean doShuffle;
    private ArrayList player;
    private ArrayList mapName;
    private String compoName;
    private boolean teamMode;
    public int cupSliderVal;
    private String roundNameShort[];
    private String roundNameLong[];

}