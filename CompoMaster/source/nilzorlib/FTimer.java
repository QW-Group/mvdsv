// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:18:38
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   FTimer.java

package nilzorlib.diverse;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

// Referenced classes of package nilzorlib.diverse:
//            FArrayList

public class FTimer
{
    class TimerThread extends Thread
    {

        public void run()
        {
            do
            {
                try
                {
                    Thread.sleep(delay);
                }
                catch(InterruptedException interruptedexception) { }
                if(!killed)
                {
                    int size = listeners.size();
                    for(int i = 0; i < size; i++)
                    {
                        ActionListener l = (ActionListener)listeners.get(i);
                        l.actionPerformed(new ActionEvent(timerObject, i, "Timer done"));
                    }

                }
            } while(repeats && !killed);
            done = true;
        }

        public void kill()
        {
            killed = true;
        }

        private boolean killed;

        TimerThread()
        {
            killed = false;
            ((Thread)this).start();
        }
    }


    public FTimer(int delay, ActionListener listener)
    {
        done = false;
        repeats = true;
        listeners = new FArrayList(3);
        listeners.add(listener);
        this.delay = delay;
        timerObject = this;
    }

    public void addActionListener(ActionListener a)
    {
        listeners.add(a);
    }

    public void setRepeats(boolean repeats)
    {
        this.repeats = repeats;
    }

    public void start()
    {
        if(theTimer == null)
            theTimer = new TimerThread();
    }

    public void restart()
    {
        stop();
        start();
    }

    public void stop()
    {
        if(theTimer != null)
        {
            theTimer.kill();
            theTimer = null;
        }
    }

    public boolean isRunning()
    {
        if(theTimer == null)
            return false;
        else
            return !done;
    }

    private int delay;
    private FArrayList listeners;
    private boolean done;
    private boolean repeats;
    private TimerThread theTimer;
    private FTimer timerObject;





}