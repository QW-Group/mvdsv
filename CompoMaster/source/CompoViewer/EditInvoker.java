// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:13:15
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   EditInvoker.java

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
                    editUrl = String.valueOf(String.valueOf(editUrl)).concat("&");
                String id = data.matchList.findMatchId(m);
                editUrl = String.valueOf(editUrl) + String.valueOf("&mid=".concat(String.valueOf(String.valueOf(id))));
                editUrl = String.valueOf(editUrl) + String.valueOf("&cid=".concat(String.valueOf(String.valueOf(compoId))));
                editUrl = String.valueOf(editUrl) + String.valueOf("&edit=".concat(String.valueOf(String.valueOf(m.isPlayed() ? 0 : 1))));
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
            String t = s.substring(0, i);
            t = String.valueOf(String.valueOf(t)).concat("___");
            t = String.valueOf(t) + String.valueOf(s.substring(i + 3));
            return t;
        }
    }

    String editUrl;
    String compoId;
    Data data;
    Applet theApplet;
}