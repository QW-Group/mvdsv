// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:15:57
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   DataInterface.java

package Data;

import java.io.*;
import java.util.ArrayList;
import nilzorlib.diverse.StringTools;

// Referenced classes of package Data:
//            Data, CupStructure, Team, RealPlayer, 
//            LeagueGroups, Match, LeagueGroup, DeathMatch, 
//            DMRound, Player, Group, MatchList

public class DataInterface
{

    public DataInterface()
    {
    }

    public static void main(String argv[])
    {
        outStream = System.out;
        inStream = new BufferedReader(new InputStreamReader(System.in));
        init(argv);
        readLoop();
    }

    protected static void init(String argv[])
    {
        outStream.print(String.valueOf(String.valueOf(Data.VERSION)).concat("\r\n"));
        if(argv != null && argv.length >= 1)
        {
            if(argv[0].toUpperCase().equals("-H"))
            {
                displaySyntax();
            } else
            {
                filename = argv[0];
                String extFile = null;
                if(argv.length >= 2)
                    extFile = argv[1];
                load(filename, extFile);
            }
        } else
        {
            data = null;
            filename = null;
            noFileWarning();
        }
        outStream.print("Type HELP for a list of all commands\r\n");
        runFlag = true;
    }

    protected static void noFileWarning()
    {
        outStream.println("No file loaded. Use the LOAD command to open a compo file.\r\n");
    }

    protected static void displaySyntax()
    {
        outStream.println("Syntax: DataInterface <datafile> <linked datafile>\r\n  Input (from stdin):\r\n  -------------------------------------------------------------------------------------\r\n".concat(String.valueOf(String.valueOf(helpStr))));
        System.exit(0);
    }

    protected static void load(String filename, String linked)
    {
        Data data_t = Data.load(filename);
        if(data_t != null)
        {
            data = data_t;
            filename = filename;
            outStream.println("Loaded main file: ".concat(String.valueOf(String.valueOf(filename))));
            if(linked != null && (data.matchList instanceof CupStructure))
            {
                Data edata = Data.load(linked);
                if(edata != null)
                {
                    outStream.println("Loaded linked data file: ".concat(String.valueOf(String.valueOf(linked))));
                    data.setExtData(edata);
                }
            }
        } else
        {
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Error: Unable to load data file ")).append(filename).append("\r\n"))));
        }
    }

    public static void readLoop()
    {
        do
        {
            try
            {
                if(!runFlag)
                    break;
                outStream.print("> ");
                String line = inStream.readLine();
                value = null;
                int idx = line.indexOf(' ');
                String command;
                if(idx == -1)
                {
                    command = line;
                } else
                {
                    command = line.substring(0, idx);
                    value = StringTools.split(':', line.substring(idx + 1), true);
                }
                command = command.toUpperCase();
                outStream.print("\r\n");
                boolean validDataCommand = false;
                try
                {
                    cmdSyntax = "";
                    if(data != null)
                    {
                        validDataCommand = true;
                        if(command.equals("ADD_LMATCH"))
                            add_lmatch();
                        else
                        if(command.equals("AUTOSCHEDULE"))
                            autoschedule();
                        else
                        if(command.equals("SHOWDISABLED"))
                            showDisabled();
                        else
                        if(command.equals("INITSHUFFLE"))
                            initshuffle();
                        else
                        if(command.equals("STRIP"))
                            strip();
                        else
                        if(command.equals("SCHEDULE"))
                            schedule();
                        else
                        if(command.equals("SETRANK"))
                            setRank();
                        else
                        if(command.equals("LISTD"))
                            listDemos();
                        else
                        if(command.equals("LISTMAPS"))
                            listMaps();
                        else
                        if(command.equals("LISTG"))
                            listGroup();
                        else
                        if(command.equals("LISTGM"))
                            listGM();
                        else
                        if(command.equals("DELRESULT"))
                            delResult();
                        else
                        if(command.equals("ADDRESULT"))
                            addResult();
                        else
                        if(command.equals("ADDDEMO"))
                            addDemo();
                        else
                        if(command.equals("CURRENTFILE"))
                            currentFile();
                        else
                        if(command.equals("CLEAR_MAPLIST"))
                            clear_maplist();
                        else
                        if(command.equals("ADDMAP"))
                            addmap();
                        else
                        if(command.equals("SETPLAYER"))
                            setplayer();
                        else
                        if(command.equals("ENABLEP"))
                            enablePlayer(true);
                        else
                        if(command.equals("DISABLEP"))
                            enablePlayer(false);
                        else
                        if(command.equals("SETWALKOVER"))
                            setWalkover();
                        else
                        if(command.equals("SETISPLAYED"))
                            setIsPlayed();
                        else
                        if(command.equals("ALLOWUSERREPORTING"))
                            setAllowUserReporting();
                        else
                        if(command.equals("GLM"))
                            generateLeagueMatches();
                        else
                        if(command.equals("EXTERNALPLAYERS"))
                            externalPlayers();
                        else
                        if(command.equals("MODE_FFA"))
                            setModeFFA();
                        else
                        if(command.equals("MODE_LEAGUE"))
                            setModeLeague();
                        else
                        if(command.equals("ADDTEAM"))
                            addTeam();
                        else
                        if(command.equals("ADDPLAYER"))
                            addPlayer();
                        else
                        if(command.equals("SETCOMPONAME"))
                            setCompoName();
                        else
                        if(command.equals("DELPLAYER_I"))
                            delPlayerByIndex();
                        else
                        if(command.equals("DELPLAYER_N"))
                            delPlayerByName();
                        else
                        if(command.equals("LIST"))
                            listPlayers();
                        else
                        if(command.equals("LISTP"))
                            listPlayers();
                        else
                        if(command.equals("LISTM"))
                            listMatches();
                        else
                            validDataCommand = false;
                    }
                    if(!validDataCommand)
                        if(command.equals("HELP"))
                            help();
                        else
                        if(command.equals("SAVE"))
                            save();
                        else
                        if(command.equals("LOAD"))
                            load();
                        else
                        if(command.equals("Q"))
                            exit(false);
                        else
                        if(command.equals("X"))
                        {
                            exit(true);
                        } else
                        {
                            outStream.print(String.valueOf(String.valueOf((new StringBuffer("Unknown command '")).append(command).append("'\r\n"))));
                            if(data == null)
                                outStream.print(String.valueOf(String.valueOf((new StringBuffer("Note that only the commands ")).append(nonDataCommands).append(" are valid when no file is loaded\r\n"))));
                        }
                }
                catch(NumberFormatException e)
                {
                    System.out.print("Input error\r\n".concat(String.valueOf(String.valueOf(cmdSyntax))));
                }
                continue;
            }
            catch(IOException ioexception) { }
            break;
        } while(true);
    }

    protected static void load()
    {
        if(!enoughArgs(1))
            return;
        String extFile = null;
        if(value.length > 1)
            extFile = value[1];
        load(value[0], extFile);
    }

    protected static boolean save()
    {
        if(value != null && value.length > 0)
            filename = value[0];
        if(filename == null)
        {
            outStream.print("No filename is specified! Please specify a filename with the 'SAVE filename' command.\r\n");
            return false;
        } else
        {
            Data.save(filename, data);
            outStream.print(String.valueOf(String.valueOf((new StringBuffer("Saved file to: ")).append(filename).append("\r\n"))));
            return true;
        }
    }

    protected static void exit(boolean saveFirst)
    {
        if(saveFirst)
        {
            boolean ok = save();
            if(!ok)
            {
                outStream.print("Cancelling quit command\r\n");
                return;
            }
            outStream.print("Quitting...\r\n");
        } else
        {
            outStream.print("Quitting without saving changes\r\n");
        }
        try
        {
            inStream.close();
            outStream.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
        data = null;
        filename = null;
        runFlag = false;
    }

    protected static void currentFile()
    {
        if(filename != null)
            outStream.print(String.valueOf(String.valueOf((new StringBuffer("Current loaded file is: ")).append(filename).append("\r\n"))));
        else
            outStream.print("No file currently loaded\r\n");
    }

    protected static void listPlayers()
    {
        int numP = data.getNumPlayers();
        for(int i = 0; i < numP; i++)
        {
            Player p = data.getPlayer(i);
            outStream.println(String.valueOf(String.valueOf((new StringBuffer(String.valueOf(String.valueOf(i)))).append(":").append(StringTools.stripQ3Colors(p.toString(), doStrip)))));
        }

    }

    protected static void listMaps()
    {
        int num = data.getNumMaps();
        outStream.print("Map list:\r\n");
        for(int i = 0; i < num; i++)
        {
            String map = data.getMapName(i);
            outStream.print(String.valueOf(String.valueOf(map)).concat("\r\n"));
        }

    }

    protected static void listMatches()
    {
        int numM = data.matchList.getNumMatches();
        for(int i = 0; i < numM; i++)
        {
            Match m = data.matchList.getMatch(i);
            outStream.print(String.valueOf(String.valueOf((new StringBuffer(String.valueOf(String.valueOf(i)))).append(":").append(StringTools.stripQ3Colors(m.toString(), doStrip)).append("\r\n"))));
        }

    }

    protected static void delPlayerByIndex()
    {
        try
        {
            if(!data.removePlayer(Integer.parseInt(value[0])))
                outStream.println("Error: No such player index");
        }
        catch(NumberFormatException e)
        {
            outStream.println("Error: Numberformatexception");
        }
    }

    protected static void delPlayerByName()
    {
        if(value.length < 1)
        {
            outStream.println("Error:2: No such player");
        } else
        {
            boolean done = false;
            Player p;
            for(int i = 0; (p = data.getPlayer(i)) != null && !done; i++)
            {
                RealPlayer q;
                if((q = p.getRealPlayer()) == null || !q.getName().equals(value[0]))
                    continue;
                String pwd;
                if(value.length > 1)
                    pwd = value[1];
                else
                    pwd = "";
                if(pwd.equals(q.getPassword()))
                {
                    data.removePlayer(i);
                    done = true;
                    outStream.println(String.valueOf(String.valueOf((new StringBuffer("Player ")).append(i).append(" deleted successfully."))));
                } else
                {
                    outStream.println("Error:1: Wrong password");
                }
            }

            if(!done)
                outStream.println("Error:2: No such player");
        }
    }

    protected static void setCompoName()
    {
        if(value != null && value.length > 0)
        {
            data.setCompoName(value[0]);
            outStream.println("Name set: ".concat(String.valueOf(String.valueOf(value[0]))));
        } else
        {
            outStream.println("Error: Too few arguments");
        }
    }

    protected static void setModeFFA()
    {
        data.setModeFFA();
        data.isLaunched = false;
        outStream.println("FFA mode set");
    }

    protected static void setModeLeague()
    {
        if(!enoughArgs(3))
        {
            return;
        } else
        {
            cmdSyntax = "MODE_LEAGUE allowDraws(true/false):pointsVictory:pointsDraw # Sets league mode\r\n";
            data.setModeLeague(Boolean.getBoolean(value[0]), Integer.parseInt(value[1]), Integer.parseInt(value[2]));
            data.isLaunched = false;
            outStream.println("League mode set");
            return;
        }
    }

    protected static void addTeam()
    {
        if(value.length < 1)
        {
            outStream.println("Error: Too few arguments");
        } else
        {
            String params[] = new String[6];
            params[2] = "9";
            for(int i = 0; i < value.length && i < 6; i++)
                params[i] = value[i];

            int r = 9;
            try
            {
                r = Integer.parseInt(params[2]);
            }
            catch(NumberFormatException numberformatexception) { }
            data.addPlayer(new Team(params[0], params[1], r, params[3], params[4], params[5]));
        }
    }

    protected static void addPlayer()
    {
        int rank = Data.MAXRANK;
        if(value.length > 2)
            try
            {
                rank = Integer.parseInt(value[2]);
            }
            catch(NumberFormatException numberformatexception) { }
        if(value.length > 0)
            data.addPlayer(new RealPlayer(value[0], value[1], rank));
    }

    protected static void externalPlayers()
    {
        data.setInDataFilename(value[0]);
        outStream.println("External players set to: Filename:".concat(String.valueOf(String.valueOf(value[0]))));
    }

    protected static void generateLeagueMatches()
    {
        if(!(data.matchList instanceof LeagueGroups))
        {
            outStream.println("Error: Datafile not a league");
        } else
        {
            ((LeagueGroups)data.matchList).generateMatches();
            outStream.println("Matches generated");
        }
    }

    protected static void setAllowUserReporting()
    {
        if(value == null)
            outStream.println("Current setting: ".concat(String.valueOf(String.valueOf(data.playerReporting))));
        else
            try
            {
                if(Integer.parseInt(value[0]) > 0)
                {
                    data.playerReporting = true;
                    outStream.println("User reporting of match results ENABLED");
                } else
                {
                    data.playerReporting = false;
                    outStream.println("User reporting of match results DISABLED");
                }
            }
            catch(NumberFormatException e)
            {
                outStream.println("Error: Parameter needs to be a number");
            }
    }

    protected static void showDisabled()
    {
        Data.showDisabled = !Data.showDisabled;
        outStream.println("ShowDisabled is now: ".concat(String.valueOf(String.valueOf(Data.showDisabled))));
    }

    protected static void initshuffle()
    {
        if(data.doShuffle)
            data.doShuffle = false;
        else
            data.doShuffle = true;
        outStream.println("DoShuffle is now: ".concat(String.valueOf(String.valueOf(data.doShuffle))));
    }

    protected static void setRank()
    {
        int r = -1;
        int i = -1;
        if(!enoughArgs(2))
            return;
        try
        {
            r = Integer.parseInt(value[1]);
            i = Integer.parseInt(value[0]);
        }
        catch(NumberFormatException e)
        {
            outStream.println("Error: Number format exeception");
            return;
        }
        data.getPlayer(i).setRank(r);
        outStream.println("Rank set for ".concat(String.valueOf(String.valueOf(data.getPlayer(i).getName()))));
    }

    protected static void schedule()
    {
        Match m = null;
        String sched = "";
        if(data.matchList instanceof LeagueGroups)
        {
            if(!enoughArgs(3))
                return;
            LeagueGroup g = (LeagueGroup)getMatch(value[0]);
            try
            {
                int mn = Integer.parseInt(value[1]);
                m = g.getMatch(mn);
                sched = value[2];
            }
            catch(NumberFormatException e)
            {
                outStream.println("Match number format error");
            }
        } else
        {
            if(!enoughArgs(2))
                return;
            m = getMatch(value[0]);
            sched = value[1];
        }
        if(m != null)
            m.scheduled = sched;
        else
            outStream.println("No match found at index");
    }

    protected static void resetMatch(DeathMatch m)
    {
        if(m != null)
        {
            m.deleteRounds();
            m.deleteDemos();
            m.isPlayed = false;
            outStream.println("Matchresult reset for match: ".concat(String.valueOf(String.valueOf(m.getName()))));
        } else
        {
            outStream.println("No such match");
        }
    }

    protected static void addResult()
    {
        if(data.matchList instanceof LeagueGroups)
        {
            if(!enoughArgs(5))
                return;
            try
            {
                LeagueGroup g = (LeagueGroup)data.matchList.getMatch(Integer.parseInt(value[0]));
                if(g == null)
                {
                    outStream.println("No such group");
                    return;
                }
                DeathMatch m = g.getMatch(Integer.parseInt(value[1]));
                if(m != null)
                    add_result(m, value[2], value[3], value[4]);
                else
                    outStream.println("No such match");
            }
            catch(NumberFormatException e)
            {
                outStream.println("Error: Number format exception");
            }
        } else
        if(data.matchList instanceof CupStructure)
        {
            if(!enoughArgs(4))
                return;
            DeathMatch m = (DeathMatch)getMatch(value[0]);
            if(m != null)
                add_result(m, value[1], value[2], value[3]);
            else
                outStream.println("No such match");
        } else
        {
            outStream.println("Tournament not of league or cup style!");
            return;
        }
    }

    protected static void add_result(DeathMatch m, String r1, String r2, String map)
    {
        try
        {
            m.addRound(new DMRound(Integer.parseInt(r1), Integer.parseInt(r2), map));
            m.calculateScore();
            m.isPlayed = true;
            outStream.println("Result added to match: ".concat(String.valueOf(String.valueOf(m.getName()))));
        }
        catch(NumberFormatException e)
        {
            outStream.println("Error: Number format exception");
        }
    }

    protected static void delResult()
    {
        if(data.matchList instanceof CupStructure)
        {
            if(!enoughArgs(1))
                return;
            DeathMatch m = (DeathMatch)getMatch(value[0]);
            if(m != null)
                resetMatch(m);
        } else
        if(data.matchList instanceof LeagueGroups)
        {
            if(!enoughArgs(2))
                return;
            try
            {
                LeagueGroup g = (LeagueGroup)data.matchList.getMatch(Integer.parseInt(value[0]));
                if(g == null)
                {
                    outStream.println("No such group");
                    return;
                }
                resetMatch(g.getMatch(Integer.parseInt(value[1])));
            }
            catch(NumberFormatException e)
            {
                outStream.println("Error: Numberformatexception");
            }
        } else
        {
            outStream.println("Tournament not of league or cup style!");
            return;
        }
    }

    protected static boolean enoughArgs(int num)
    {
        if(value == null || value.length < num)
        {
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Not enough parameters. (")).append(num).append(" needed)"))));
            return false;
        } else
        {
            return true;
        }
    }

    protected static void help()
    {
        outStream.println(helpStr);
    }

    protected static void autoschedule()
    {
        if(!(data.matchList instanceof LeagueGroups))
            outStream.println("Tournament not a league");
        else
        if(!enoughArgs(1))
        {
            outStream.println("Need to specify which group to schedule");
        } else
        {
            int maxPass = 50;
            if(value.length > 1)
                try
                {
                    maxPass = Integer.parseInt(value[1]);
                }
                catch(NumberFormatException e)
                {
                    outStream.println("Unable to interpret maxpass value. Setting to default (50)");
                }
            LeagueGroups lg = (LeagueGroups)data.matchList;
            LeagueGroup g = (LeagueGroup)lg.getMatch(Integer.parseInt(value[0]));
            g.autoSchedule(maxPass);
        }
    }

    protected static void add_lmatch()
    {
        if(!enoughArgs(3))
            return;
        if(!(data.matchList instanceof LeagueGroups))
        {
            outStream.println("Tournament not a league!");
            return;
        }
        int groupId = Integer.parseInt(value[0]);
        try
        {
            Player a = data.getPlayer(Integer.parseInt(value[1]));
            Player b = data.getPlayer(Integer.parseInt(value[2]));
            if(a == null || b == null)
                throw new Exception();
            DeathMatch m = new DeathMatch(a, b);
            LeagueGroup g = (LeagueGroup)data.matchList.getMatch(groupId);
            g.addMatch(m);
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Added match: \"")).append(m).append("\""))));
        }
        catch(Exception e)
        {
            outStream.println("Invalid team specified.");
        }
    }

    protected static void addDemo()
    {
        if(data.matchList instanceof CupStructure)
        {
            if(!enoughArgs(4))
                return;
            try
            {
                DeathMatch m = (DeathMatch)data.matchList.getMatch(Integer.parseInt(value[0]));
                if(m != null)
                {
                    if(value.length >= 5)
                        m.deleteDemos();
                    m.addDemo(value[1], value[2], value[3]);
                    outStream.println("Demo added to: ".concat(String.valueOf(String.valueOf(m.toString()))));
                } else
                {
                    outStream.println("No such match");
                }
            }
            catch(NumberFormatException e)
            {
                outStream.println("Error:Numberformat");
            }
        } else
        if(data.matchList instanceof LeagueGroups)
        {
            if(!enoughArgs(5))
                return;
            try
            {
                LeagueGroup g = (LeagueGroup)data.matchList.getMatch(Integer.parseInt(value[0]));
                if(g == null)
                {
                    outStream.println("No such group");
                    return;
                }
                DeathMatch m = g.getMatch(Integer.parseInt(value[1]));
                if(m != null)
                {
                    if(value.length >= 6)
                        m.deleteDemos();
                    m.addDemo(value[2], value[3], value[4]);
                    outStream.println("Demo added to: ".concat(String.valueOf(String.valueOf(m.toString()))));
                } else
                {
                    outStream.println("No such match");
                }
            }
            catch(NumberFormatException e)
            {
                outStream.println("Error:Numberformat");
            }
        } else
        {
            outStream.println("Tournament not of league or cup style!");
            return;
        }
    }

    protected static void clear_maplist()
    {
        data.clearMapList();
        outStream.println("Map list erased.");
    }

    protected static void addmap()
    {
        if(!enoughArgs(1))
        {
            return;
        } else
        {
            data.addMap(value[0]);
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Map \"")).append(value[0]).append("\" added"))));
            return;
        }
    }

    protected static void setplayer()
    {
        if(!enoughArgs(1))
            return;
        int i = -1;
        try
        {
            i = Integer.parseInt(value[0]);
        }
        catch(NumberFormatException numberformatexception) { }
        RealPlayer p;
        if((p = (RealPlayer)data.getPlayer(i)) == null)
        {
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Error: No such player (")).append(value[0]).append(")"))));
            return;
        }
        if(!value[1].equals(""))
            p.setName(value[1]);
        if(value.length > 2 && !value[2].equals(""))
            p.setPassword(value[2]);
        try
        {
            if(value.length > 4 && !value[3].equals(""))
                p.setRank(Integer.parseInt(value[3]));
        }
        catch(NumberFormatException numberformatexception1) { }
        if(value.length > 4 && !value[4].equals(""))
            p.setEmail(value[4]);
        outStream.println("New playerdata set:\r\n".concat(String.valueOf(String.valueOf(p.toString()))));
    }

    protected static void enablePlayer(boolean state)
    {
        if(!enoughArgs(1))
            return;
        int i = -1;
        try
        {
            i = Integer.parseInt(value[0]);
        }
        catch(NumberFormatException numberformatexception) { }
        RealPlayer p;
        if((p = (RealPlayer)data.getPlayer(i)) == null)
        {
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Error: No such player (")).append(value[0]).append(")"))));
            return;
        } else
        {
            p.disabled = !state;
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Disabled: ")).append(!state).append("\r\n").append(p))));
            return;
        }
    }

    protected static void setWalkover()
    {
        int pi = 0;
        int i = 0;
        LeagueGroup group = null;
        DeathMatch m = null;
        boolean done = false;
        try
        {
            if(data.matchList instanceof LeagueGroups)
            {
                if(!enoughArgs(3))
                    return;
                int gi = Integer.parseInt(value[i++]);
                int mi = Integer.parseInt(value[i++]);
                pi = Integer.parseInt(value[i++]);
                group = (LeagueGroup)data.matchList.getMatch(gi);
                if(group != null)
                    m = group.getMatch(mi);
            } else
            if(data.matchList instanceof CupStructure)
            {
                if(!enoughArgs(2))
                    return;
                int mi = Integer.parseInt(value[i++]);
                pi = Integer.parseInt(value[i++]);
                m = (DeathMatch)data.matchList.getMatch(mi);
            }
            if(value.length > i)
            {
                int frags = Integer.parseInt(value[i++]);
                if(m != null)
                {
                    m.setWalkover(pi, frags);
                    done = true;
                }
            } else
            if(m != null)
            {
                m.setWalkover(pi);
                done = true;
            }
        }
        catch(NumberFormatException numberformatexception) { }
        catch(ClassCastException classcastexception) { }
        if(done)
            outStream.println("Walkover set successfully.");
        else
            outStream.println("Error: Wrong match- or player index or compo not of cup or league structure.");
    }

    protected static void strip()
    {
        doStrip = !doStrip;
        outStream.println("Strip: ".concat(String.valueOf(String.valueOf(doStrip))));
    }

    protected static void setIsPlayed()
    {
        if(!enoughArgs(2))
            return;
        Match m;
        if((m = getMatch(value[0])) == null)
            return;
        if(value[1].equals("1"))
        {
            m.setPlayed(true);
            outStream.println(String.valueOf(String.valueOf(m)).concat("> isPlayed = TRUE"));
        } else
        {
            m.setPlayed(false);
            outStream.println(String.valueOf(String.valueOf(m)).concat("> isPlayed = FALSE"));
        }
    }

    protected static void listDemos()
    {
        if(data.matchList instanceof LeagueGroups)
        {
            LeagueGroup g;
            for(int i = 0; (g = (LeagueGroup)data.matchList.getMatch(i)) != null; i++)
            {
                g = (LeagueGroup)data.matchList.getMatch(i);
                Match m;
                for(int j = 0; (m = (Match)g.matchList.get(j)) != null; j++)
                {
                    String s = m.demoString(String.valueOf(String.valueOf((new StringBuffer("")).append(i).append(":").append(j).append(":"))));
                    if(s != null)
                        outStream.println(s);
                }

            }

        } else
        {
            Match m;
            for(int j = 0; (m = data.matchList.getMatch(j)) != null; j++)
            {
                String s = m.demoString(String.valueOf(String.valueOf((new StringBuffer("")).append(j).append(":"))));
                if(s != null)
                    outStream.println(s);
            }

        }
    }

    protected static void listGroup()
    {
        if(!enoughArgs(1))
        {
            inputError();
            return;
        }
        Match m;
        if((m = getMatch(value[0])) == null)
        {
            inputError();
            return;
        }
        if(!(m instanceof Group))
        {
            outStream.println("Match not a group");
            return;
        } else
        {
            outStream.println(((Group)m).stringStandings());
            return;
        }
    }

    protected static void listGM()
    {
        if(!enoughArgs(1))
        {
            inputError();
            return;
        }
        Match m;
        if((m = getMatch(value[0], 0)) == null)
        {
            inputError();
            return;
        }
        if(!(m instanceof LeagueGroup))
        {
            outStream.println("Match not a leaguegroup");
            return;
        } else
        {
            outStream.println(((LeagueGroup)m).stringMatches());
            return;
        }
    }

    protected static void inputError()
    {
        outStream.println("Syntax error");
    }

    protected static Match getMatch(String s)
    {
        return getMatch(s, 0);
    }

    protected static Match getMatch(String s, int add)
    {
        int i = -1;
        Match m;
        try
        {
            i = Integer.parseInt(s) + add;
            m = data.matchList.getMatch(i);
        }
        catch(NumberFormatException e)
        {
            outStream.println(String.valueOf(String.valueOf((new StringBuffer("Error: Cannot convert string \"")).append(s).append("\" to a number"))));
            Match match = null;
            return match;
        }
        if(m == null)
            outStream.println("Error: No match at index ".concat(String.valueOf(String.valueOf(i))));
        return m;
    }

    protected static final int ADDPLAYER = 0;
    protected static final int EDITPLAYER = 1;
    protected static String cmdSyntax = "";
    protected static BufferedReader inStream;
    protected static PrintStream outStream;
    protected static String filename;
    protected static boolean isLoaded = false;
    protected static boolean runFlag = true;
    protected static Data data;
    protected static String value[];
    protected static boolean doStrip = true;
    protected static String nonDataCommands = "LOAD, SAVE, X, Q and HELP";
    protected static String helpStr = "  ADD_LMATCH groupIdx:team-1-id:team-2-id       # Adds match to league group\r\n  ADDDEMO [groupIdx:]matchNr:url:pov:round:bool.isFirst\r\n  ADDRESULT [groupIdx:]matchNr:score1:score2:mapName\r\n    # Match Result\r\n  ADDMAP mapName \r\n  ADDPLAYER playername:password:rank\r\n  ADDTEAM teamname:password:rank:shortname:hp:irc\r\n  ADDTEAMMEMBER teamIndex:membername   # Find the index by using LIST\r\n  AUTOSCHEDULE groupIdx[:Max_Passes]     # Tries to build a league schedule\r\n  CURRENTFILE # Echos the filename of the currently loaded file\r\n  CLEAR_MAPLIST \r\n  DELRESULT [groupIdx:]matchIndex      # Delete match results\r\n  DELPLAYER_I playerindex              # Delete player by index.\r\n  DELPLAYER_N playerName:Password      # Delete player by name and password\r\n  DISABLEP playerIndex                 # Withdraws player from compo\r\n  ENABLEP playerIndex                  # Returns player to compo\r\n  EXTERNALPLAYERS filename:url # Set the filename source of the external player source\r\n  GLM # Generate internal League Matches (resets all match data!!)\r\n  HELP # This text\r\n  INITSHUFFLE # (G) Toggles wether or not the teams/players are to be shuffled initially\r\n  LOAD mainFile[:linkedFile] # Loads a new compo file \r\n  LIST # List indexes, names and other info of all registered players\r\n  LISTD # List all demos in format [groupIdx:]matchIdx:Url:Pov:RoundNr\r\n  LISTG groupIdx # List group data\r\n  LISTGM groupIdx # List group match data\r\n  LISTM  # List matches\r\n  LISTMAPS # List map pool \r\n  MODE_FFA # Sets FFA mode\r\n  MODE_LEAGUE allowDraws(true/false):pointsVictory:pointsDraw # Sets league mode\r\n  SAVE [filename] # Saves the file. If no name specified, last name used\r\n  SCHEDULE [group:]matchIndex:date(String)\r\n  SETCOMPONAME name\r\n  SETRANK index:rank\r\n  SETPLAYER index:[playername]:[password]:[rank]:[email]\r\n  SETISPLAYED matchIndex:(0/1) # Sets \"isPlayed\"-flag true or false\r\n  SETWALKOVER matchIndex:playerNr[:Frags] # playerNr = 0/1\r\n  SHOWDISABLED # Toggles viewing of disabled team/players on/off\r\n  STRIP   # toggles stripping of q3 color codes. Default = 1\r\n  X / END # Saves file and exits \r\n  Q       # Quits without saving \r\n";

}