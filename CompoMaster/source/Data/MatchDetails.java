/**

$Id: MatchDetails.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import com.borland.jbcl.layout.XYConstraints;
import com.borland.jbcl.layout.XYLayout;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.EventObject;

// Referenced classes of package Data:
//            Team, DoubleRound, Data, DeathMatch, 
//            Match, MatchList, Player, Group, 
//            EditListener

public class MatchDetails extends Panel
    implements ActionListener
{

    MatchDetails(DeathMatch m, Container parent, EditListener el, Data data)
    {
        match = m;
        this.el = el;
        this.data = data;
        Dimension d = getDimension();
        setLayout(new XYLayout());
        ok = new Button("OK");
        ok.addActionListener(this);
        add(ok, new XYConstraints(d.width - 75, 5, 70, 20));
        if(Data.doDetailsButton)
        {
            details = new Button("Details");
            details.addActionListener(this);
            add(details, new XYConstraints(d.width - 75, 30, 70, 20));
        }
        doLayout();
    }

    public DeathMatch getMatch()
    {
        return match;
    }

    public void paint(Graphics g)
    {
        if(match == null)
            return;
        Point offset = new Point(0, 0);
        Dimension d = getDimension();
        int nr;
        if((nr = match.getNumRounds()) > 0)
            d.height += nr * 20;
        if((nr = match.getNumDemos()) > 0)
            d.height += (3 + nr) * 20;
        if(match.getPlayer(0) instanceof Team)
        {
            Team t = (Team)match.getPlayer(0);
            int maxMemb = t.getMemberCount();
            t = (Team)match.getPlayer(1);
            if(t.getMemberCount() > maxMemb)
                maxMemb = t.getMemberCount();
            if(maxMemb == 0)
                maxMemb = 1;
            d.height += (3 + maxMemb) * 20;
        }
        if(g == null)
            return;
        Point corner = new Point(offset.x, offset.y);
        int startX;
        int x = startX = 5;
        int startY;
        int y = startY = 20 + corner.y;
        g.setFont(MatchList.getMainFont());
        FontMetrics fm = g.getFontMetrics(MatchList.getMainFont());
        g.setColor(BGCOL1);
        g.fill3DRect(corner.x, corner.y, d.width, d.height, true);
        int tmp = Data.MAXLETTERS;
        Data.MAXLETTERS = 40;
        g.setColor(DeathMatch.INFO_TEXTCOL);
        String s = new String("^3Match code: ^7".concat(((Match) (match)).name));
        Data.drawName(g, s, 5, y);
        y += 20;
        g.setColor(DeathMatch.INFO_TEXTCOL);
        if(!match.isPlayed())
        {
            s = new String("^3Scheduled: ^7".concat(String.valueOf(((Match) (match)).scheduled)));
        } else
        {
            String result = new String((new StringBuffer(match.getScore(0))).append(" - ").append(match.getScore(1)));
            int wo;
            if((wo = match.getWalkover()) != -1)
            {
                result = "";
                Data.drawName(g, match.getPlayer(1 - wo).getName().concat("^7 wins on walkover"), x, y + 20, -1);
            }
            s = new String("^3Result:^7 ".concat(result));
        }
        Data.drawName(g, s, 5, y, -1);
        y += 40;
        g.setColor(BGCOL2);
        g.fill3DRect(4, y - 15, d.width - 8, 25, true);
        g.setColor(Group.headerCol);
        s = new String((new StringBuffer(match.getPlayer(0).getName())).append("^7 vs ").append(match.getPlayer(1).getName()));
        Data.drawName(g, s, 5, y, -1);
        Data.MAXLETTERS = tmp;
        y += 20;
        if(match.isPlayed())
        {
            int C1 = 8;
            int centerX = d.width / 2;
            int numRounds = match.getNumRounds();
            g.setColor(BGCOL3);
            if(data.doRounds)
                g.fill3DRect(4, y, d.width - 8, (int)((double)40 * ((double)numRounds + 0.25D)), true);
            else
                g.fill3DRect(4, y, d.width - 8, (int)((double)20 * ((double)numRounds + 0.5D)), true);
            y += 20;
            g.setColor(DeathMatch.INFO_TEXTCOL);
            int digits = 3;
            for(int i = 0; i < numRounds; i++)
            {
                g.setColor(HEADERCOL);
                if(data.doRounds)
                {
                    g.drawString(match.getMap(i), C1, y + 10);
                    g.setColor(TEXTCOL);
                    g.drawString("-", centerX - 3, y);
                    g.drawString("-", centerX - 3, y + 20);
                    DoubleRound r = (DoubleRound)match.getRound(i);
                    MatchList.drawNumber(r.getScore(0, 1), digits, centerX + 10, y, g);
                    g.drawString(new String((new StringBuffer("(")).append(data.getRoundNameShort(r.getSideIndex(0, 1))).append(")")), centerX + 35, y);
                    MatchList.drawNumber(r.getScore(0, 0), digits, centerX - 27, y, g);
                    g.drawString(new String((new StringBuffer("(")).append(data.getRoundNameShort(r.getSideIndex(0, 0))).append(")")), centerX - 60, y);
                    MatchList.drawNumber(r.getScore(1, 1), digits, centerX + 10, y + 20, g);
                    g.drawString(new String((new StringBuffer("(")).append(data.getRoundNameShort(r.getSideIndex(1, 1))).append(")")), centerX + 35, y + 20);
                    MatchList.drawNumber(r.getScore(1, 0), digits, centerX - 27, y + 20, g);
                    g.drawString(new String((new StringBuffer("(")).append(data.getRoundNameShort(r.getSideIndex(1, 0))).append(")")), centerX - 60, y + 20);
                    y += 40;
                } else
                {
                    g.drawString(match.getMap(i), C1, y);
                    g.setColor(TEXTCOL);
                    g.drawString("-", centerX - 3, y);
                    MatchList.drawNumber(match.getFrags(i, 1), digits, centerX + 10, y, g);
                    MatchList.drawNumber(match.getFrags(i, 0), digits, centerX - 27, y, g);
                    y += 20;
                }
            }

        }
    }

    public Dimension getDimension()
    {
        int width = 300;
        int height = 100;
        int numRounds = 0;
        if(match != null)
        {
            numRounds = match.getNumRounds();
            if(numRounds > 0)
                if(data.doRounds)
                    height += 40 * numRounds + 20;
                else
                    height += 20 * (numRounds + 1);
            if((match.getPlayer(0) instanceof Team) && (match.getPlayer(1) instanceof Team))
            {
                int maxMembers = 0;
                Team t = (Team)match.getPlayer(0);
                maxMembers = t.getMemberCount();
                t = (Team)match.getPlayer(1);
                if(t.getMemberCount() > maxMembers)
                    maxMembers = t.getMemberCount();
                height += 20 * (maxMembers + 2);
            }
        }
        return new Dimension(width, height);
    }

    public void actionPerformed(ActionEvent e)
    {
        if(e.getSource() == ok)
            removeMe();
        if(e.getSource() == details)
        {
            if(el != null)
                el.editMatch(match);
            removeMe();
        }
    }

    public void setEditListener(EditListener el)
    {
        this.el = el;
    }

    private void removeMe()
    {
        setVisible(false);
        match = null;
        try
        {
            parent.doLayout();
            parent.repaint();
        }
        catch(NullPointerException nullpointerexception) { }
    }

    public static final int EDIT_WIDTH = 45;
    public static final int EDIT_HEIGHT = 20;
    public static final int INNER_MARGIN = 4;
    public static final Color TEXTCOL;
    public static final Color HEADERCOL;
    public static final int COLWIDTH = 180;
    public static final Color BGCOL1 = new Color(50, 50, 50);
    public static final Color BGCOL2 = new Color(100, 80, 80);
    public static final Color BGCOL3 = new Color(80, 100, 80);
    public static final int SCOREW = 30;
    public static final int LA = 20;
    private EditListener el;
    private DeathMatch match;
    private Button details;
    private Button ok;
    private Container parent;
    private Data data;

    static 
    {
        TEXTCOL = Color.white;
        HEADERCOL = Color.yellow;
    }
}