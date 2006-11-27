/**

$Id: MatchClickListener.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

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