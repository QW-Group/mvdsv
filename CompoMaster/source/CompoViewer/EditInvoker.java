/**

$Id: EditInvoker.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

package CompoViewer;

import Data.*;
import java.applet.Applet;
import java.applet.AppletContext;
import java.net.MalformedURLException;
import java.net.URL;

public class EditInvoker
    implements EditListener
{

    public EditInvoker(String url, String compoId, Data d, Applet a)
    {
        editUrl = url;
        this.compoId = compoId;
        theApplet = a;
        data = d;
    }

    public void editMatch(Match match)
    {
        DeathMatch m = (DeathMatch)match;
        if(editUrl != null)
            try
            {
                int idx = editUrl.indexOf('?');
                if(idx != -1 && idx != editUrl.length() - 1)
                    editUrl = editUrl.concat("&");
                String id = data.matchList.findMatchId(m);
                editUrl = editUrl.concat("&mid=").concat(String.valueOf(id));
                editUrl = editUrl.concat("&cid=").concat(String.valueOf(compoId));
                editUrl = editUrl.concat("&edit=").concat(String.valueOf(m.isPlayed() ? 0 : 1));
                theApplet.getAppletContext().showDocument(new URL(editUrl));
            }
            catch(MalformedURLException e)
            {
                theApplet.getAppletContext().showStatus("EditURL not an URL");
            }
        else
            theApplet.getAppletContext().showStatus("No EditURL defined in <applet> tag");
    }

    private String hideUrl(String s)
    {
        int i = s.indexOf("://");
        if(i == -1)
        {
            return s;
        } else
        {
            return s.substring(0, i).concat("___").concat(s.substring(i + 3));
        }
    }

    String editUrl;
    String compoId;
    Data data;
    Applet theApplet;
}
