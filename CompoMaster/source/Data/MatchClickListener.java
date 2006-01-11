// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:49
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   MatchClickListener.java

package Data;

import java.awt.event.MouseEvent;
import java.util.EventListener;

// Referenced classes of package Data:
//            Match

public interface MatchClickListener
    extends EventListener
{

    public abstract void matchClick(Match match, MouseEvent mouseevent);
}