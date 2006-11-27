/**

$Id: RealPlayer.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.awt.Color;
import java.io.*;
import nilzorlib.diverse.StringTools;

// Referenced classes of package Data:
//			Player, Data, Team, DeathMatch

public class RealPlayer extends Player
{

	public RealPlayer(String name, String password, int rank)
	{
		email = null;
		disabled = false;
		this.password = password;
		this.name = name;
		super.rank = rank;
	}

	public RealPlayer(String name, String password)
	{
		email = null;
		disabled = false;
		this.password = password;
		this.name = name;
		super.rank = Data.MAXRANK;
	}

	public RealPlayer(String name)
	{
		email = null;
		disabled = false;
		this.name = name;
		password = null;
		super.rank = Data.MAXRANK;
	}

	public boolean isDisabled()
	{
		if(Data.debug)
			System.out.println(new String((new StringBuffer("	  RPL-ISDISABLED [")).append(getName()).append("]")));
		return disabled;
	}

	public boolean isWalkover()
	{
		if(Data.debug)
			System.out.println(new String((new StringBuffer("	  RPL-ISWALKOVER [")).append(getName()).append("]")));
		return isDisabled();
	}

	public RealPlayer getRealPlayer()
	{
		return this;
	}

	public String getPassword()
	{
		return password;
	}

	public void setPassword(String s)
	{
		password = s;
	}

	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	public String getEmail()
	{
		return email;
	}

	public void setEmail(String email)
	{
		this.email = email;
	}

	public Color getColor()
	{
		if(isDisabled())
			return DeathMatch.M_WO_COL;
		else
			return DeathMatch.M_BGCOL1;
	}

	public String toString()
	{
		return new String((new StringBuffer(name)).append(":").append(password).append(":").append(super.rank).append(":").append(email));
	}

	public static Data importPlayerList(BufferedReader inFile)
	{
		boolean isTeam = false;
		Data data = new Data();
		try
		{
			Team t = null;
			RealPlayer p = null;
			do
			{
				String s;
				if((s = inFile.readLine()) == null || s.startsWith("[playerlist]"))
					break;
				if(!s.startsWith("[teamdef]"))
					continue;
				t = new Team("");
				isTeam = true;
				break;
			} while(true);
			if(isTeam)
			{
				do
				{
					String s;
					if((s = inFile.readLine()) == null)
						break;
					if(s.startsWith("[teamdef]"))
					{
						if(t != null)
							data.addPlayer(t);
						t = new Team("");
					}
					String values[];
					if((values = StringTools.eqSplit("name", '&', s)) != null)
						t.setName(values[0]);
					if((values = StringTools.eqSplit("shortname", '&', s)) != null)
						t.shortname = values[0];
					if((values = StringTools.eqSplit("hp", '&', s)) != null)
						t.hp = values[0];
					if((values = StringTools.eqSplit("irc", '&', s)) != null)
						t.irc = values[0];
					if((values = StringTools.eqSplit("password", '&', s)) != null)
						t.password = values[0];
					if((values = StringTools.eqSplit("rank", '&', s)) != null)
						t.rank = Integer.parseInt(values[0]);
					if((values = StringTools.eqSplit("disabled", '&', s)) != null && values[0].equals("1"))
						t.disabled = true;
					if((values = StringTools.eqSplit("members", '&', s)) != null)
					{
						int i = 0;
						while(i < values.length) 
						{
							t.addMember(values[i]);
							i++;
						}
					}
				} while(true);
				if(t != null)
					data.addPlayer(t);
			} else
			{
				do
				{
					String s;
					if((s = inFile.readLine()) == null)
						break;
					if(s.startsWith("[playerdef]"))
					{
						if(p != null)
							data.addPlayer(p);
						p = new RealPlayer("");
					}
					String values[];
					if((values = StringTools.eqSplit("name", '&', s)) != null)
						p.setName(values[0]);
					if((values = StringTools.eqSplit("password", '&', s)) != null)
						p.setPassword(values[0]);
					if((values = StringTools.eqSplit("rank", '&', s)) != null)
						p.setRank(Integer.parseInt(values[0]));
				} while(true);
				if(p != null)
					data.addPlayer(p);
			}
			Data data1 = data;
			return data1;
		}
		catch(IOException e)
		{
			System.out.println("IO Exception");
			Data data2 = null;
			return data2;
		}
	}

	public static void exportPlayerList(Data data, FileWriter outFile)
	{
		try
		{
			if(data.getTeamMode())
			{
				for(int i = 0; data.getPlayer(i) != null; i++)
				{
					Team team = (Team)data.getPlayer(i);
					outFile.write("[teamdef]\r\n");
					outFile.write(new String((new StringBuffer("name=")).append(team.getName()).append("\r\n")));
					if(team.shortname != null)
						outFile.write(new String((new StringBuffer("shortname=")).append(team.shortname).append("\r\n")));
					if(((RealPlayer) (team)).password != null)
						outFile.write(new String((new StringBuffer("password=")).append(((RealPlayer) (team)).password).append("\r\n")));
					if(team.hp != null)
						outFile.write(new String((new StringBuffer("hp=")).append(team.hp).append("\r\n")));
					if(team.irc != null)
						outFile.write(new String((new StringBuffer("irc=")).append(team.irc).append("\r\n")));
					outFile.write(new String((new StringBuffer("rank=")).append(((Player) (team)).rank).append("\r\n")));
					if(team.getMemberCount() > 0)
					{
						outFile.write("members=");
						int j;
						for(j = 0; j < team.getMemberCount() - 1; j++)
							outFile.write(team.getMember(j).concat("&"));

						outFile.write(team.getMember(j).concat("\r\n"));
					}
				}

			} else
			{
				outFile.write("[playerlist]\r\n");
				int team = data.getNumPlayers();
				for(int i = 0; i < team; i++)
				{
					outFile.write("[playerdef]\r\nname=");
					outFile.write(((RealPlayer)data.getPlayer(i)).name.concat("\r\n"));
					outFile.write("rank=");
					outFile.write(String.valueOf(((RealPlayer)data.getPlayer(i)).rank).concat("\r\n"));
				}

			}
			outFile.flush();
			outFile.close();
		}
		catch(IOException e)
		{
			System.out.println("IO Error");
		}
	}

	private static final long serialVersionUID = 0xa73a4423539a76ebL;
	protected String password;
	protected String name;
	protected String email;
	public boolean disabled;

}