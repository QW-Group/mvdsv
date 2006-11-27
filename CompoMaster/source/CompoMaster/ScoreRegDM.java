/**

$Id: ScoreRegDM.java,v 1.2 2006/11/27 15:15:46 vvd0 Exp $

**/

package CompoMaster;

import Data.*;
import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.EventObject;
import javax.swing.*;
import javax.swing.text.JTextComponent;

// Referenced classes of package CompoMaster:
//            CompoMaster

public class ScoreRegDM extends JDialog
    implements ActionListener
{
    class RoleBox extends JPanel
        implements MouseListener
    {

        public void paintComponent(Graphics g)
        {
            super.paintComponent(g);
            int role = firstRole[roundIdx];
            if(doSwap)
                role = 1 - role;
            String txt = cmData.getRoundNameShort(role);
            txtWidth = g.getFontMetrics().stringWidth(txt);
            g.setColor(Color.black);
            g.drawString(txt, (ScoreRegDM.roleDim.width - txtWidth) / 2, ScoreRegDM.roleDim.height - 3);
        }

        public void mousePressed(MouseEvent e)
        {
            al.actionPerformed(new ActionEvent(this, roundIdx, ""));
        }

        public void mouseReleased(MouseEvent mouseevent)
        {
        }

        public void mouseClicked(MouseEvent mouseevent)
        {
        }

        public void mouseEntered(MouseEvent mouseevent)
        {
        }

        public void mouseExited(MouseEvent mouseevent)
        {
        }

        private int txtWidth;
        private ActionListener al;
        private int roundIdx;
        private Data cmData;
        private boolean doSwap;

        RoleBox(Data cmData, int roundIdx, boolean doSwap, ActionListener al)
        {
            txtWidth = -1;
            this.al = al;
            this.roundIdx = roundIdx;
            this.cmData = cmData;
            this.doSwap = doSwap;
            setPreferredSize(ScoreRegDM.roleDim);
            setMinimumSize(ScoreRegDM.roleDim);
            setMaximumSize(ScoreRegDM.roleDim);
            setBackground(Color.white);
            setBorder(BorderFactory.createLineBorder(Color.black));
            addMouseListener(this);
        }
    }

    class KeyListener extends KeyAdapter
        implements FocusListener
    {

        public void keyPressed(KeyEvent e)
        {
            Object src = e.getSource();
            if(src == txtTotal1 || src == txtTotal2)
                btnStatPlayed.setSelected(true);
            if(e.getKeyCode() == 10)
            {
                if(src == txtScheduled)
                    txtTotal1.grabFocus();
                if(src == txtTotal1)
                    txtTotal2.grabFocus();
            }
            if(e.getSource() != txtTotal1)
                if(e.getSource() != txtTotal2);
        }

        public void focusGained(FocusEvent e)
        {
            JTextField txt = (JTextField)e.getSource();
            txt.setSelectionStart(0);
            txt.setSelectionEnd(txt.getText().length());
        }

        public void focusLost(FocusEvent focusevent)
        {
        }

        KeyListener()
        {
        }
    }


    public ScoreRegDM(DeathMatch match, JFrame parent)
    {
        super(parent);
        btnOk = new JButton("Done");
        btnCancel = new JButton("Cancel");
        btnMoreRounds = new JButton("More");
        btnLessRounds = new JButton("Less");
        btnStatPending = new JToggleButton("Pending");
        btnStatDefaulted = new JToggleButton("Defaulted");
        btnStatPlayed = new JToggleButton("Played");
        numRounds = 0;
        disableAllBtns = false;
        txtFrag2 = new JTextField[20];
        txtFrag1 = new JTextField[20];
        cmbMap = new JComboBox[10];
        firstRole = new int[10];
        pnlAllRounds = new JPanel();
        lblScore = new JLabel();
        txtTotal1 = new JTextField();
        txtTotal2 = new JTextField();
        txtScheduled = new JTextField();
        kListener = new KeyListener();
        this.match = match;
        data = CompoMaster.getDataInstance();
        if(thisFrame != null)
            thisFrame.dispose();
        thisFrame = this;
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
        setTitle("Edit match ".concat(match.getName()));
        txtPlayer1 = new JTextField(match.getPlayer(0).getName());
        txtPlayer2 = new JTextField(match.getPlayer(1).getName());
        txtPlayer1.setPreferredSize(new Dimension(150, 25));
        txtPlayer2.setPreferredSize(new Dimension(150, 25));
        txtScheduled.setPreferredSize(new Dimension(320, 25));
        txtTotal1.setPreferredSize(new Dimension(150, 25));
        txtTotal2.setPreferredSize(new Dimension(150, 25));
        btnWinA = new JToggleButton(new String((new StringBuffer(txtPlayer1.getText())).append(" wins")));
        btnWinB = new JToggleButton(new String((new StringBuffer(txtPlayer2.getText())).append(" wins")));
        btnWinA.setPreferredSize(new Dimension(150, 25));
        btnWinB.setPreferredSize(new Dimension(150, 25));
        txtTotal1.setText(String.valueOf(match.getScore(0)));
        txtTotal2.setText(String.valueOf(match.getScore(1)));
        txtScheduled.setText(((Match) (match)).scheduled);
        GridBagLayout layout = new GridBagLayout();
        cp = getContentPane();
        cp.setLayout(layout);
        ButtonGroup statusButtons = new ButtonGroup();
        statusButtons.add(btnStatPending);
        statusButtons.add(btnStatDefaulted);
        statusButtons.add(btnStatPlayed);
        if(!match.canBePlayed())
            btnStatPlayed.setEnabled(false);
        if(match.getPlayer(0).isWalkover() || match.getPlayer(1).isWalkover())
        {
            btnStatPlayed.setEnabled(false);
            btnStatDefaulted.setEnabled(false);
            btnStatPending.setEnabled(false);
            btnWinA.setEnabled(false);
            btnWinB.setEnabled(false);
            btnLessRounds.setEnabled(false);
            btnMoreRounds.setEnabled(false);
            disableAllBtns = true;
        }
        if(match.isPlayed() && match.getWalkover() == -1)
            btnStatPlayed.setSelected(true);
        else
        if(match.isPlayed())
        {
            btnStatDefaulted.setSelected(true);
            if(match.getWalkover() == 1)
                btnWinA.setSelected(true);
            else
                btnWinB.setSelected(true);
        } else
        {
            btnStatPending.setSelected(true);
        }
        addGridRow(layout, new JLabel("Teams:"), txtPlayer1, new JLabel("vs"), txtPlayer2);
        JPanel pnlStatusButtons = new JPanel();
        pnlStatusButtons.add(btnStatPending);
        pnlStatusButtons.add(btnStatDefaulted);
        pnlStatusButtons.add(btnStatPlayed);
        addGridRow(layout, new JLabel("Match status:"), pnlStatusButtons);
        addGridRow(layout, new JLabel("Scheduled:"), txtScheduled);
        pnlScoreA = new JPanel();
        pnlScoreB = new JPanel();
        setScoreLine();
        addGridRow(layout, lblScore, pnlScoreA, new JLabel("-"), pnlScoreB);
        JPanel pnlConfirm = new JPanel();
        ButtonGroup winButtons = new ButtonGroup();
        winButtons.add(btnWinA);
        winButtons.add(btnWinB);
        if(data.doMaps)
        {
            layRounds = new GridBagLayout();
            pnlAllRounds.setLayout(layRounds);
            JPanel pnlRoundCtl = new JPanel();
            pnlRoundCtl.add(btnLessRounds);
            pnlRoundCtl.add(btnMoreRounds);
            cp = pnlAllRounds;
            addGridRow(layRounds, new JLabel("Map:"), new JLabel(txtPlayer1.getText()), new JLabel(""), new JLabel(txtPlayer2.getText()));
            cp = getContentPane();
            int i = 0;
            for(i = 0; i < match.getNumRounds(); i++)
                addRound(match.getRound(i));

            if(i == 0)
                for(i = 0; i < startRounds; i++)
                    addRound();

            spAllRounds = new JScrollPane(pnlAllRounds);
            spAllRounds.setPreferredSize(new Dimension(300, 120));
            pnlRoundFrame = new JPanel(new BorderLayout());
            pnlRoundFrame.setBorder(BorderFactory.createTitledBorder("Maps:"));
            pnlRoundFrame.add(spAllRounds, "Center");
            pnlRoundFrame.add(pnlRoundCtl, "North");
            addGridRow(layout, pnlRoundFrame, 1, 10, 1.0F);
        }
        pnlConfirm.add(btnOk);
        pnlConfirm.add(btnCancel);
        addGridRow(layout, pnlConfirm);
        addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e)
            {
                cancelPressed();
            }

        });
        btnOk.addActionListener(this);
        btnCancel.addActionListener(this);
        btnStatPending.addActionListener(this);
        btnStatDefaulted.addActionListener(this);
        btnStatPlayed.addActionListener(this);
        btnMoreRounds.addActionListener(this);
        btnLessRounds.addActionListener(this);
        txtScheduled.addKeyListener(kListener);
        txtTotal1.addKeyListener(kListener);
        txtTotal2.addKeyListener(kListener);
        txtTotal1.addFocusListener(kListener);
        txtTotal2.addFocusListener(kListener);
        pack();
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        setLocation((screen.width - getWidth()) / 2, (screen.height - getHeight()) / 2);
    }

    private void addRound()
    {
        DMRound round;
        if(data.doRounds)
            round = new DoubleRound(data.getMapName(0));
        else
            round = new DMRound(data.getMapName(0));
        addRound(round);
        checkRoundCount(1);
    }

    private void addRound(DMRound round)
    {
        JPanel mapPanel = new JPanel();
        mapPanel.add(new JLabel(new String((new StringBuffer(numRounds + 1)).append(": "))));
        cmbMap[numRounds] = new JComboBox(data.getMapList().toArray());
        mapPanel.add(cmbMap[numRounds]);
        cmbMap[numRounds].setSelectedItem(round.getMapName());
        cmbMap[numRounds].setPreferredSize(new Dimension(120, 21));
        JPanel fragPanel1 = new JPanel();
        JPanel fragPanel2 = new JPanel();
        if(data.doRounds)
        {
            DoubleRound dround = (DoubleRound)round;
            int idx = numRounds * 2;
            FlowLayout fLay = new FlowLayout(1, 2, 2);
            txtFrag1[idx] = new JTextField(String.valueOf(dround.getScore(0, 0)));
            txtFrag2[idx] = new JTextField(String.valueOf(dround.getScore(0, 1)));
            txtFrag1[idx + 1] = new JTextField(String.valueOf(dround.getScore(1, 0)));
            txtFrag2[idx + 1] = new JTextField(String.valueOf(dround.getScore(1, 1)));
            Dimension dim1 = new Dimension(40, 21);
            txtFrag1[idx].setPreferredSize(dim1);
            txtFrag2[idx].setPreferredSize(dim1);
            txtFrag1[idx + 1].setPreferredSize(dim1);
            txtFrag2[idx + 1].setPreferredSize(dim1);
            JPanel upLeft = new JPanel(fLay);
            JPanel loLeft = new JPanel(fLay);
            JPanel upRight = new JPanel(fLay);
            JPanel loRight = new JPanel(fLay);
            firstRole[numRounds] = dround.getSideIndex(0, 0);
            upLeft.add(txtFrag1[idx]);
            upLeft.add(new RoleBox(data, numRounds, false, this));
            loLeft.add(txtFrag1[idx + 1]);
            loLeft.add(new RoleBox(data, numRounds, true, this));
            upRight.add(new RoleBox(data, numRounds, true, this));
            upRight.add(txtFrag2[idx]);
            loRight.add(new RoleBox(data, numRounds, false, this));
            loRight.add(txtFrag2[idx + 1]);
            fragPanel1.setLayout(new BorderLayout(0, 0));
            fragPanel2.setLayout(new BorderLayout(0, 0));
            fragPanel1.add(upLeft, "North");
            fragPanel1.add(loLeft, "South");
            fragPanel2.add(upRight, "North");
            fragPanel2.add(loRight, "South");
        } else
        {
            txtFrag1[numRounds] = new JTextField(String.valueOf(round.getScore(0)));
            txtFrag2[numRounds] = new JTextField(String.valueOf(round.getScore(1)));
            Dimension dim1 = new Dimension(40, 21);
            txtFrag1[numRounds].setPreferredSize(dim1);
            txtFrag2[numRounds].setPreferredSize(dim1);
            fragPanel1.add(txtFrag1[numRounds]);
            fragPanel2.add(txtFrag2[numRounds]);
        }
        cp = pnlAllRounds;
        addGridRow(layRounds, mapPanel, fragPanel1, new JLabel("-"), fragPanel2);
        cp = getContentPane();
        numRounds++;
        validate();
        if(spAllRounds != null)
            spAllRounds.repaint();
    }

    private void setScoreLine()
    {
        boolean show = !btnStatDefaulted.isSelected();
        Component c = null;
        boolean changes = false;
        if(pnlScoreA.getComponentCount() > 0)
            c = pnlScoreA.getComponent(0);
        if(c != btnWinA && !show)
        {
            pnlScoreA.removeAll();
            pnlScoreB.removeAll();
            pnlScoreA.add(btnWinA);
            pnlScoreB.add(btnWinB);
            changes = true;
        } else
        if(c != txtTotal1 && show)
        {
            pnlScoreA.removeAll();
            pnlScoreB.removeAll();
            pnlScoreA.add(txtTotal1);
            pnlScoreB.add(txtTotal2);
            changes = true;
        }
        if(changes)
        {
            pnlScoreA.repaint();
            pnlScoreB.repaint();
            validate();
        }
    }

    private void removeRound()
    {
        numRounds--;
        int j = pnlAllRounds.getComponentCount();
        for(int i = -1; i > -5; i--)
            pnlAllRounds.remove(j + i);

        validate();
        if(spAllRounds != null)
            spAllRounds.repaint();
        checkRoundCount(-1);
    }

    private void checkRoundCount(int dif)
    {
        if(dif < 0 && numRounds == 0)
            btnLessRounds.setEnabled(false);
        else
        if(!disableAllBtns)
            btnLessRounds.setEnabled(true);
        if(dif > 0 && numRounds == 10)
            btnMoreRounds.setEnabled(false);
        else
        if(!disableAllBtns)
            btnMoreRounds.setEnabled(true);
    }

    private void addGridRow(GridBagLayout gb, Component description, Component left, Component center, Component right)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.weightx = 0.20000000000000001D;
        gb.setConstraints(description, c);
        cp.add(description);
        c.weightx = 0.34999999999999998D;
        gb.setConstraints(left, c);
        cp.add(left);
        c.weightx = 0.10000000000000001D;
        gb.setConstraints(center, c);
        cp.add(center);
        c.weightx = 0.34999999999999998D;
        c.gridwidth = 0;
        gb.setConstraints(right, c);
        cp.add(right);
    }

    private void addGridRow(GridBagLayout gb, Component only)
    {
        addGridRow(gb, only, 0, 10, 0.0F);
    }

    private void addGridRow(GridBagLayout gb, Component only, int fill, int anchor, float horWeight)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.gridwidth = 0;
        c.fill = fill;
        c.anchor = anchor;
        c.weighty = horWeight;
        gb.setConstraints(only, c);
        cp.add(only);
    }

    private void addGridRow(GridBagLayout gb, Component description, Component center)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.weightx = 0.20000000000000001D;
        gb.setConstraints(description, c);
        cp.add(description);
        c.weightx = 0.80000000000000004D;
        c.gridwidth = 0;
        gb.setConstraints(center, c);
        cp.add(center);
    }

    private void donePressed()
    {
        int a = 0;
        int b = 0;
        match.scheduled = txtScheduled.getText();
        if(btnStatPlayed.isSelected())
        {
            try
            {
                a = Integer.parseInt(txtTotal1.getText());
                b = Integer.parseInt(txtTotal2.getText());
            }
            catch(NumberFormatException e)
            {
                txtTotal1.setText("?");
                txtTotal2.setText("?");
                return;
            }
            match.deleteRounds();
            if(data.doMaps)
            {
                for(int i = 0; i < numRounds; i++)
                {
                    String mapName = (String)cmbMap[i].getSelectedItem();
                    try
                    {
                        DMRound d;
                        if(data.doRounds)
                        {
                            int idx = i + i;
                            int fa1 = Integer.parseInt(txtFrag1[idx].getText());
                            int fb1 = Integer.parseInt(txtFrag2[idx].getText());
                            int fa2 = Integer.parseInt(txtFrag1[idx + 1].getText());
                            int fb2 = Integer.parseInt(txtFrag2[idx + 1].getText());
                            d = new DoubleRound(fa1, fb1, fa2, fb2, firstRole[i], mapName);
                        } else
                        {
                            int fa = Integer.parseInt(txtFrag1[i].getText());
                            int fb = Integer.parseInt(txtFrag2[i].getText());
                            d = new DMRound(fa, fb, mapName);
                        }
                        match.addRound(d);
                    }
                    catch(NumberFormatException e)
                    {
                        JOptionPane.showMessageDialog(this, new JLabel("Check that all round scores are numerical."), "Number format error", 0);
                        return;
                    }
                }

            }
            match.setScore(0, a);
            match.setScore(1, b);
            match.setIsPlayed(true);
        } else
        if(btnStatDefaulted.isSelected())
        {
            if(btnWinA.isSelected())
                match.setWalkover(1);
            else
                match.setWalkover(0);
        } else
        if(btnStatPending.isSelected())
            match.setIsPlayed(false);
        startRounds = numRounds;
        data.isChanged = true;
        data.matchList.updateImage();
        dispose();
        CompoMaster.frame3.repaint();
    }

    private void cancelPressed()
    {
        dispose();
    }

    private void updateRoundPanel()
    {
    }

    private boolean storeRound()
    {
        return false;
    }

    public void actionPerformed(ActionEvent e)
    {
        Object src = e.getSource();
        if(src == btnOk)
            donePressed();
        if(src == btnCancel)
            cancelPressed();
        if(src == btnStatPending || src == btnStatPlayed || src == btnStatDefaulted)
            setScoreLine();
        if(src == btnMoreRounds)
            addRound();
        if(src == btnLessRounds)
            removeRound();
        if(e.getActionCommand() == "")
            swapRounds(e.getID());
    }

    private void swapRounds(int roundIdx)
    {
        firstRole[roundIdx] = 1 - firstRole[roundIdx];
        pnlAllRounds.repaint();
    }

    protected static JDialog thisFrame;
    protected static Dimension roleDim = new Dimension(20, 20);
    private static final String ROLEBOX_MSG = "";
    JButton btnOk;
    JButton btnCancel;
    JButton btnMoreRounds;
    JButton btnLessRounds;
    JToggleButton btnWinA;
    JToggleButton btnWinB;
    JToggleButton btnStatPending;
    JToggleButton btnStatDefaulted;
    JToggleButton btnStatPlayed;
    int numRounds;
    private static int startRounds = 2;
    private boolean disableAllBtns;
    JTextField txtFrag2[];
    JTextField txtFrag1[];
    JComboBox cmbMap[];
    int firstRole[];
    JPanel pnlScoreA;
    JPanel pnlScoreB;
    JPanel pnlAllRounds;
    JPanel pnlRoundFrame;
    JScrollPane spAllRounds;
    JLabel lblScore;
    JTextField txtTotal1;
    JTextField txtTotal2;
    JTextField txtPlayer1;
    JTextField txtPlayer2;
    JTextField txtScheduled;
    GridBagLayout layRounds;
    KeyListener kListener;
    DeathMatch match;
    Container cp;
    private Data data;


}