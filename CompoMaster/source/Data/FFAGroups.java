// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:33
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   FFAGroups.java

package Data;

import java.awt.Graphics;
import java.awt.Point;
import java.util.ArrayList;

// Referenced classes of package Data:
//            GroupStructure, FFAGroup, Match, Group, 
//            Data

public class FFAGroups extends GroupStructure
{

    public boolean canBePlayed(Match m)
    {
        return true;
    }

    public FFAGroups(Data data)
    {
        super(data);
    }

    public void addGroup(int size, String name)
    {
        FFAGroup f = new FFAGroup(size);
        f.setName(name);
        super.groupList.add(f);
        if(size > super.largestGroupSize)
            super.largestGroupSize = size;
    }

    public void initCompo()
    {
        commonInit();
    }

    public void drawHighlight(Graphics g1, Point point)
    {
    }

    public Match getMatch(int matchNr)
    {
        return (FFAGroup)super.groupList.get(matchNr);
    }

    public Match getMatchAt(Point p)
    {
        int x = p.x;
        int y = p.y;
        for(int i = 0; i < super.groupList.size(); i++)
        {
            FFAGroup g = (FFAGroup)super.groupList.get(i);
            int groupHeight = g.mPlayer.length * 20 + 44;
            if(y > ((Group) (g)).yPos && y < ((Group) (g)).yPos + groupHeight && x > ((Group) (g)).xPos && x < ((Group) (g)).xPos + 240)
                return g;
        }

        return null;
    }

    public String findMatchId(Match target)
    {
        Match m;
        for(int i = 0; (m = getMatch(i)) != null; i++)
            if(m == target)
                return "".concat(String.valueOf(String.valueOf(i)));

        return null;
    }

    private static final long serialVersionUID = 0x557f9c2cae8af85aL;

}