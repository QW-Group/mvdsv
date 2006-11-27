/**

$Id: JCompoDrawing.java,v 1.2 2006/11/27 15:15:45 vvd0 Exp $

**/

package CompoMaster;

import Data.Data;
import Data.MatchList;
import java.awt.Graphics;
import javax.swing.JComponent;
import javax.swing.JPanel;

public class JCompoDrawing extends JPanel
{

    public JCompoDrawing(Data data)
    {
        this.data = data;
        data.matchList.setMyContainer(this);
    }

    public void paintComponent(Graphics g)
    {
        super.paintComponent(g);
        g.drawImage(data.matchList.getImage(), 0, 0, this);
    }

    Data data;
    Graphics g;
}