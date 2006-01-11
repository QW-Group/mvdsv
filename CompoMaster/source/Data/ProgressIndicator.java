// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:17:14
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   ProgressIndicator.java

package Data;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class ProgressIndicator
{

    public ProgressIndicator(int goal)
    {
        progress = 0;
        this.goal = goal;
    }

    public ProgressIndicator()
    {
        progress = 0;
    }

    public void setGoal(int g)
    {
        goal = g;
    }

    public void setProgress(int i)
    {
        progress = i;
        if(al != null)
            al.actionPerformed(new ActionEvent(this, 0, null));
    }

    public int getProgress()
    {
        return progress;
    }

    public int getGoal()
    {
        return goal;
    }

    public void setEventHandler(ActionListener al)
    {
        this.al = al;
    }

    private ActionListener al;
    private int goal;
    private int progress;
}