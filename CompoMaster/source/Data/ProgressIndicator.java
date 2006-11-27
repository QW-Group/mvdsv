/**

$Id: ProgressIndicator.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

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