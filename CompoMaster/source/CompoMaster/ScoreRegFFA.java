/**

$Id: ScoreRegFFA.java,v 1.2 2006/11/27 15:15:46 vvd0 Exp $

**/

package CompoMaster;

import Data.*;
import com.borland.jbcl.layout.XYConstraints;
import com.borland.jbcl.layout.XYLayout;
import java.awt.*;
import java.awt.event.*;
import java.util.EventObject;
import javax.swing.*;
import javax.swing.text.JTextComponent;

// Referenced classes of package CompoMaster:
//            CompoMaster

public class ScoreRegFFA extends JDialog
    implements ActionListener
{
    class KeyListener extends KeyAdapter
    {

        public void keyPressed(KeyEvent e)
        {
            if(e.getSource() == txtScore)
            {
                chkIsPlayed.setSelected(true);
                if(e.getKeyCode() == 10)
                    nextPressed();
            }
        }

        KeyListener()
        {
        }
    }


    public ScoreRegFFA(FFAGroup match, JFrame parent)
    {
        super(parent);
        xYLayout1 = new XYLayout();
        lblPlayer = new JLabel();
        txtScore = new JTextField();
        jLabel1 = new JLabel();
        jLabel2 = new JLabel();
        btnNext = new JButton();
        lblCounter = new JLabel();
        txtOk = new JButton();
        jLabel3 = new JLabel();
        txtScheduled = new JTextField();
        chkIsPlayed = new JCheckBox();
        kListener = new KeyListener();
        btnDone = new JButton();
        if(thisFrame != null)
            thisFrame.dispose();
        thisFrame = this;
        this.match = match;
        try
        {
            jbInit();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    private void jbInit()
        throws Exception
    {
        getContentPane().setLayout(xYLayout1);
        jLabel1.setText("Player:");
        jLabel2.setText("Score:");
        btnNext.setText("Next");
        lblCounter.setText("1/x");
        xYLayout1.setHeight(250);
        xYLayout1.setWidth(366);
        setTitle("FFA Group ");
        lblPlayer.setBackground(Color.white);
        lblPlayer.setBorder(BorderFactory.createLineBorder(Color.black));
        lblPlayer.setRequestFocusEnabled(false);
        jLabel3.setText("Scheduled:");
        chkIsPlayed.setText("Match is played");
        btnDone.setText("Done");
        getContentPane().add(txtScore, new XYConstraints(205, 91, 48, 28));
        getContentPane().add(jLabel3, new XYConstraints(12, 22, 82, 24));
        getContentPane().add(lblCounter, new XYConstraints(20, 93, 32, 29));
        getContentPane().add(jLabel1, new XYConstraints(64, 64, 54, 24));
        getContentPane().add(lblPlayer, new XYConstraints(64, 92, 104, 27));
        getContentPane().add(jLabel2, new XYConstraints(205, 62, 60, 26));
        getContentPane().add(btnNext, new XYConstraints(267, 93, 69, 29));
        getContentPane().add(chkIsPlayed, new XYConstraints(66, 131, 141, 23));
        getContentPane().add(txtScheduled, new XYConstraints(85, 23, 229, -1));
        getContentPane().add(btnDone, new XYConstraints(118, 171, 125, 28));
        setTitle("Edit ".concat(match.getName()));
        txtScheduled.setText(((Match) (match)).scheduled);
        chkIsPlayed.setSelected(match.isPlayed());
        curPlayer = 0;
        showPlayer(curPlayer);
        txtScore.addKeyListener(kListener);
        btnNext.addActionListener(this);
        btnDone.addActionListener(this);
        pack();
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        setLocation((screen.width - getWidth()) / 2, (screen.height - getHeight()) / 2);
    }

    private void showPlayer(int nr)
    {
        lblCounter.setText(new String((new StringBuffer(nr + 1)).append("/").append(match.getNumPlayers())));
        lblPlayer.setText(match.getPlayer(nr).getName());
        txtScore.setText(String.valueOf(match.getScore(nr)));
        txtScore.grabFocus();
        txtScore.setSelectionStart(0);
        txtScore.setSelectionEnd(10);
    }

    private boolean nextPressed()
    {
        try
        {
            match.setScore(curPlayer, Integer.parseInt(txtScore.getText()));
        }
        catch(NumberFormatException e)
        {
            txtScore.setText("?");
            showPlayer(curPlayer);
            boolean flag = false;
            return flag;
        }
        curPlayer++;
        if(curPlayer >= match.getNumPlayers())
            curPlayer = 0;
        showPlayer(curPlayer);
        return true;
    }

    private void donePressed()
    {
        if(!nextPressed())
        {
            return;
        } else
        {
            match.setIsPlayed(chkIsPlayed.isSelected());
            match.scheduled = txtScheduled.getText();
            match.sortPlayers();
            dispose();
            Data data = CompoMaster.getDataInstance();
            data.isChanged = true;
            data.matchList.updateImage();
            CompoMaster.frame3.repaint();
            return;
        }
    }

    public void actionPerformed(ActionEvent e)
    {
        if(e.getSource() == btnDone)
            donePressed();
        if(e.getSource() == btnNext)
            nextPressed();
    }

    XYLayout xYLayout1;
    JLabel lblPlayer;
    JTextField txtScore;
    JLabel jLabel1;
    JLabel jLabel2;
    JButton btnNext;
    JLabel lblCounter;
    JButton txtOk;
    JLabel jLabel3;
    JTextField txtScheduled;
    JCheckBox chkIsPlayed;
    FFAGroup match;
    private static JDialog thisFrame;
    int curPlayer;
    KeyListener kListener;
    JButton btnDone;

}