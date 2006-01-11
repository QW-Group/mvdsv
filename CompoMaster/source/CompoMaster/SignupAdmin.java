// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:11:03
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   SignupAdmin.java

package CompoMaster;

import Data.*;
import com.borland.jbcl.layout.XYConstraints;
import com.borland.jbcl.layout.XYLayout;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.util.EventObject;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.JTextComponent;

// Referenced classes of package CompoMaster:
//            MultipleSlider, Menu, TextFieldLimiter, CompoMaster

public class SignupAdmin extends JFrame
{
    class KeyListener extends KeyAdapter
    {

        public void keyPressed(KeyEvent e)
        {
            Object src = e.getSource();
            if(src == txtChnFilename)
                chainFileChanged();
            if(src == txtNameMember)
            {
                memberEditRange = tblTeam.getSelectedRows();
                if(e.getKeyCode() == 10)
                    storeMember();
            }
            if(src == txtName || src == txtRank)
            {
                playerEditRange = tblPlayers.getSelectedRows();
                if(e.getKeyCode() == 10)
                    if(src == txtRank && playerEditRange.length > 0)
                    {
                        int newIdx = playerEditRange[playerEditRange.length - 1] + 1;
                        if(newIdx < tblPlayers.getRowCount())
                        {
                            tblPlayers.getSelectionModel().setSelectionInterval(newIdx, newIdx);
                            txtRank.setSelectionStart(0);
                            txtRank.setSelectionEnd(txtRank.getText().length());
                        } else
                        {
                            enterButtonPressed();
                        }
                    } else
                    {
                        enterButtonPressed();
                    }
                if(txtName.getText().length() >= Data.MAXLETTERS && e.getKeyCode() != 8 && e.getKeyCode() != 37)
                    dispatchEvent(new KeyEvent(txtName, 401, 0L, 0, 8));
            }
            if(src == txtMap && e.getKeyCode() == 10)
                btnAddMapPressed();
            if(src == txtRoundLong1 || e.getSource() == txtRoundLong2 || src == txtRoundShort1 || e.getSource() == txtRoundShort2)
                cmbPreDefRounds.setSelectedIndex(0);
        }

        char nr;

        KeyListener()
        {
        }
    }

    class ButtonListener
        implements ActionListener, ChangeListener
    {

        public void actionPerformed(ActionEvent e)
        {
            Object src = e.getSource();
            if(src == btnChnLoad)
                chainLoadFile();
            if(src == btnChnBrowse)
                chainBrowseFile();
            if(src == rbtMatch || src == rbtMap || src == rbtRound)
                setScoreMode();
            if(src == rbnQualify)
                showQualifyPanel(true);
            if(src == rbnStandalone)
                showQualifyPanel(false);
            if(src == rbnIndividuals || src == rbnTeams)
                setPlayerMode();
            if(src == rbnCup || src == rbnFFA || src == rbnLeague)
                updateTourneyTab();
            if(src == rbnPointMatch || src == rbnPointMap)
                toggleLeaguePointSystem(rbnPointMatch.isSelected());
            if(src == chkAllowDraws)
                txtLgDraw.setEnabled(chkAllowDraws.isSelected());
            if(src == cmbPreDefRounds)
                setPreDef(cmbPreDefRounds.getSelectedIndex());
            if(src == btnDelPlayer)
                deleteButtonPressed();
            if(src == btnClearPlayer)
                clearPlayerInput();
            if(src == btnDelMember)
                delMember();
            if(src == btnClearMember)
                clearMemberInput();
            if(src == btnRankUp)
                rankButtonPressed(-1);
            if(src == btnRankDown)
                rankButtonPressed(1);
            if(src == launchButton)
                launchCompo();
            if(src == btnAddMap)
                btnAddMapPressed();
            if(src == btnDeleteMaps)
                btnDeleteMaps();
            if(src == btnKeepMaps)
                btnKeepMaps();
            if(src == txtNumGroups)
                updateGroupTab(0);
        }

        public void stateChanged(ChangeEvent e)
        {
            Object src = e.getSource();
            if(src == sldNumGroups)
                updateGroupTab(1);
            if(src == mslElimination)
                if(mslElimination.getSelectedValue(0) <= 4)
                    setFDEFinals(false);
                else
                    setFDEFinals(true);
            if(src == mslQualify)
            {
                chainUpdateDesc();
                updateImportedPlayers();
            }
        }

        ButtonListener()
        {
        }
    }

    class PlayerTableModel extends AbstractTableModel
    {

        public void setSelectedTeam(Team t)
        {
            selectedTeam = t;
        }

        public Team getSelectedTeam()
        {
            return selectedTeam;
        }

        public void update()
        {
            fireTableRowsInserted(getRowCount(), getRowCount());
            fireTableDataChanged();
        }

        public int getRowCount()
        {
            if(data == null)
                return 0;
            else
                return data.getNumPlayers();
        }

        public int getColumnCount()
        {
            return 2;
        }

        public Object getValueAt(int row, int col)
        {
            if(data == null)
                return new Integer(0);
            if(col == 1)
            {
                int thisRank = data.getPlayer(row).getRank();
                if(thisRank == Data.MAXRANK)
                    return "N/A";
                else
                    return new Integer(data.getPlayer(row).getRank());
            } else
            {
                return data.getPlayer(row).getName();
            }
        }

        public String getColumnName(int col)
        {
            if(col == 0)
                return "Contestant:";
            else
                return new String("Ranking:");
        }

        public boolean isCellEditable(int rowIndex, int columnIndex)
        {
            return false;
        }

        public void setValueAt(Object obj, int i, int j)
        {
        }

        PlayerTableModel()
        {
        }
    }

    class MemberTableModel extends AbstractTableModel
    {

        public void update()
        {
            fireTableRowsInserted(getRowCount(), getRowCount());
            fireTableDataChanged();
        }

        public int getRowCount()
        {
            if(selectedTeam == null)
                return 0;
            else
                return selectedTeam.getMemberCount();
        }

        public int getColumnCount()
        {
            return 1;
        }

        public Object getValueAt(int row, int col)
        {
            if(selectedTeam == null)
                return "";
            else
                return selectedTeam.getMember(row);
        }

        public String getColumnName(int col)
        {
            return "Name:";
        }

        public boolean isCellEditable(int rowIndex, int columnIndex)
        {
            return false;
        }

        public void setValueAt(Object obj, int i, int j)
        {
        }

        MemberTableModel()
        {
        }
    }

    class TableListener
        implements ListSelectionListener
    {

        public void valueChanged(ListSelectionEvent e)
        {
            if(e.getSource() == tblTeam.getSelectionModel())
            {
                if(memberEditRange != null && memberEditRange.length == 1)
                    storeMember();
                int numRows = tblTeam.getSelectedRowCount();
                btnDelMember.setEnabled(true);
                if(numRows == 1)
                {
                    int row = tblTeam.getSelectedRow();
                    txtNameMember.setText((String)tblTeam.getValueAt(row, 0));
                } else
                if(numRows > 1)
                    txtNameMember.setText("");
                else
                    btnDelMember.setEnabled(false);
            } else
            {
                if(playerEditRange != null && playerEditRange.length > 0)
                {
                    storePlayerChanges();
                    playerEditRange = null;
                }
                txtNameMember.setEnabled(false);
                selectedTeam = null;
                teamBorder.setTitle("Team members - (no team selected):");
                int numRows = tblPlayers.getSelectedRowCount();
                if(numRows == 1)
                {
                    int row = tblPlayers.getSelectedRow();
                    txtName.setText((String)tblPlayers.getValueAt(row, 0));
                    txtRank.setText("".concat(String.valueOf(String.valueOf(tblPlayers.getValueAt(row, 1)))));
                    btnDelPlayer.setEnabled(true);
                    Player pl = data.getPlayer(row);
                    if(pl instanceof Team)
                    {
                        selectedTeam = (Team)pl;
                        txtNameMember.setEnabled(true);
                        teamBorder.setTitle(String.valueOf(String.valueOf((new StringBuffer("Team members - ")).append(selectedTeam.getName()).append(":"))));
                    } else
                    {
                        selectedTeam = null;
                    }
                } else
                if(numRows > 1)
                {
                    txtName.setText("");
                    txtRank.setText("N/A");
                    btnDelPlayer.setEnabled(true);
                } else
                {
                    btnDelPlayer.setEnabled(false);
                }
                teamMemberModel.update();
                pnlTeamMembers.repaint();
            }
        }

        TableListener()
        {
        }
    }

    class PFocusAdapter extends FocusAdapter
    {

        public void focusGained(FocusEvent e)
        {
            if(e.getSource() == cellTF)
                cellTF.setText("");
            if(e.getSource() == txtName && tblPlayers.getSelectedRowCount() > 1)
                tblPlayers.clearSelection();
            if(e.getSource() == txtRank)
            {
                txtRank.setSelectionStart(0);
                txtRank.setSelectionEnd(txtRank.getText().length());
            }
        }

        public void focusLost(FocusEvent e)
        {
            updateGroupTab(0);
        }

        PFocusAdapter()
        {
        }
    }

    class PColumnModel extends DefaultTableColumnModel
    {

        public TableColumn getColumn(int idx)
        {
            A.setPreferredWidth(200);
            B.setPreferredWidth(50);
            if(idx == 0)
                return A;
            else
                return B;
        }

        private TableColumn A;
        private TableColumn B;

        PColumnModel()
        {
            A = new TableColumn(0, 200);
            B = new TableColumn(1, 50);
        }
    }


    public SignupAdmin()
    {
        CHN_OUT = new Color(255, 100, 100);
        CHN_IN = new Color(100, 255, 100);
        firstTime = true;
        jTabbedPane1 = new JTabbedPane();
        pnlTourney = new JPanel();
        optionsPanel = new JPanel();
        pnlGroups = new JPanel();
        lstMaps = new JList();
        tableModel = new PlayerTableModel();
        cupTypeGroup = new ButtonGroup();
        winModeGroup = new ButtonGroup();
        cellTF = new JTextField("");
        buttonListener = new ButtonListener();
        keyListener = new KeyListener();
        newMenu = new JMenuItem();
        pnlMatch2A = new JPanel();
        btnAddMap = new JButton();
        btnDeleteMaps = new JButton();
        txtMap = new JTextField();
        xYLayout7 = new XYLayout();
        jLabel19 = new JLabel();
        pnlPlayers = new JPanel();
        lblAdvancingPlayers = new JLabel();
        borderLayout1 = new BorderLayout();
        pnlMatch1 = new JPanel();
        rbtMatch = new JRadioButton();
        rbtRound = new JRadioButton();
        jLabel8 = new JLabel();
        gridBagLayout1 = new GridBagLayout();
        gblTeamMembers = new GridBagLayout();
        rbtMap = new JRadioButton();
        lblLodDesc = new JLabel();
        matchDetailGroup = new ButtonGroup();
        gridBagMapPanel = new GridBagLayout();
        btnKeepMaps = new JButton();
        pnlMatch2 = new JPanel();
        borderLayout2 = new BorderLayout();
        spMapList1 = new JScrollPane(lstMaps);
        pnlMatch2B = new JPanel();
        gridBagLayout2 = new GridBagLayout();
        jLabel15 = new JLabel();
        cmbPreDefRounds = new JComboBox();
        jLabel16 = new JLabel();
        txtRoundLong1 = new JTextField();
        txtRoundShort1 = new JTextField();
        txtRoundLong2 = new JTextField();
        txtRoundShort2 = new JTextField();
        borderLayout3 = new BorderLayout();
        lblDummy1 = new JLabel();
        jLabel17 = new JLabel();
        pnlLaunch = new JPanel();
        launchButton = new JButton();
        borderLayout4 = new BorderLayout();
        pnlTourney1 = new JPanel();
        pnlCup = new JPanel();
        tourneyTypeGroup = new ButtonGroup();
        pnlLeague = new JPanel();
        mslElimination = new MultipleSlider();
        lblNumFinals = new JLabel();
        gridBagLayout3 = new GridBagLayout();
        pnlFiller1 = new JPanel();
        sldNumGroups = new JSlider();
        txtNumGroups = new JTextField();
        gridBagLayout4 = new GridBagLayout();
        pnlLg2 = new JPanel();
        borderLayout6 = new BorderLayout();
        txtLgVictory = new JTextField();
        txtLgDraw = new JTextField();
        jLabel3 = new JLabel();
        lblLgDraw = new JLabel();
        chkAllowDraws = new JCheckBox();
        gridBagLayout5 = new GridBagLayout();
        pnlTourney11 = new JPanel();
        pnlTourney12 = new JPanel();
        txtCompoName = new JTextField();
        jLabel10 = new JLabel();
        rbnLeague = new JRadioButton();
        rbnFFA = new JRadioButton();
        rbnCup = new JRadioButton();
        borderLayout5 = new BorderLayout();
        borderLayout7 = new BorderLayout();
        lblPlayers = new JLabel();
        lblGroups = new JLabel();
        btnBrowse = new JButton();
        playersPanelExt = new JPanel();
        jLabel14 = new JLabel();
        jLabel13 = new JLabel();
        jLabel12 = new JLabel();
        jLabel11 = new JLabel();
        txtRanks = new JTextField();
        txtExtFileName = new JTextField();
        playersPanelReal = new JPanel();
        txtNumPlayers = new JTextField();
        playersPanel = new JPanel();
        jLabel6 = new JLabel();
        xYLayout9 = new XYLayout();
        xYLayout8 = new XYLayout();
        jLabel4 = new JLabel();
        xYLayout4 = new XYLayout();
        gridBagLayout6 = new GridBagLayout();
        txtName = new JTextField(Data.MAXLETTERS);
        btnRankDown = new JButton();
        btnRankUp = new JButton();
        pnlContestants = new JPanel();
        btnDelPlayer = new JButton();
        spPlayers = new JScrollPane();
        pnlButtons1 = new JPanel();
        jLabel2 = new JLabel();
        txtRank = new JTextField();
        jLabel1 = new JLabel();
        pnlRank = new JPanel();
        btnClearPlayer = new JButton();
        spTeam = new JScrollPane();
        tblTeam = new JTable();
        teamMemberModel = new MemberTableModel();
        pnlButtons2 = new JPanel();
        pnlTeamMembers = new JPanel();
        gridBagLayout7 = new GridBagLayout();
        txtNameMember = new JTextField();
        btnClearMember = new JButton();
        btnDelMember = new JButton();
        gridBagLayout8 = new GridBagLayout();
        jLabel5 = new JLabel();
        rbnTeams = new JRadioButton();
        pnlContTop = new JPanel();
        rbnIndividuals = new JRadioButton();
        bgContestants = new ButtonGroup();
        pnlRadioButtons = new JPanel();
        borderLayout8 = new BorderLayout();
        chkQ3Colors = new JCheckBox();
        pnlChaining = new JPanel();
        borderLayout9 = new BorderLayout();
        pnlChnIn = new JPanel();
        pnlChnOut = new JPanel();
        gridBagLayout9 = new GridBagLayout();
        rbnQualify = new JRadioButton();
        rbnStandalone = new JRadioButton();
        jLabel7 = new JLabel();
        jTextArea1 = new JTextArea();
        borderLayout10 = new BorderLayout();
        bgChain = new ButtonGroup();
        mslQualify = new MultipleSlider();
        lblQualify = new JLabel();
        pnlQualify = new JPanel();
        gridBagLayout10 = new GridBagLayout();
        txtChnFilename = new JTextField();
        btnChnLoad = new JButton();
        btnChnBrowse = new JButton();
        jLabel9 = new JLabel();
        pnlDummy = new JPanel();
        lblPlayersRemaining = new JLabel();
        cmbNumFinals = new JComboBox(seFinales);
        jLabel18 = new JLabel();
        txtPlayerCount = new JTextField();
        jPanel1 = new JPanel();
        rbnPointMatch = new JRadioButton();
        rbnPointMap = new JRadioButton();
        bgPointSystem = new ButtonGroup();
        jLabel20 = new JLabel();
        rbnEqual = new JRadioButton();
        jLabel21 = new JLabel();
        rbnDivisions = new JRadioButton();
        bgSeedingSystem = new ButtonGroup();
        if(CompoMaster.getDataInstance() == null)
            CompoMaster.setDataInstance(new Data());
        data = CompoMaster.getDataInstance();
        enableEvents(64L);
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
        tblTeam = new JTable(teamMemberModel);
        tblPlayers = new JTable(tableModel);
        spPlayers = new JScrollPane(tblPlayers);
        titledBorder10 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Outgoing");
        titledBorder11 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Incoming");
        pnlChaining.setLayout(borderLayout9);
        pnlChnOut.setBorder(titledBorder10);
        pnlChnOut.setLayout(borderLayout10);
        pnlChnIn.setBorder(titledBorder11);
        pnlChnIn.setLayout(gridBagLayout9);
        rbnQualify.setEnabled(true);
        rbnQualify.setToolTipText("");
        rbnQualify.setText("Players qualify from a preceeding tournament");
        rbnStandalone.setSelected(true);
        rbnStandalone.setText("Standalone tournament");
        jLabel7.setText("File:");
        jTextArea1.setBackground(UIManager.getColor("Viewport.background"));
        jTextArea1.setMinimumSize(new Dimension(100, 17));
        jTextArea1.setText("Outgoing contestants are specified in the receivning tournament file. No file has requested this.");
        jTextArea1.setLineWrap(true);
        jTextArea1.setWrapStyleWord(true);
        lblQualify.setText("DetailedDesc");
        pnlQualify.setLayout(gridBagLayout10);
        txtChnFilename.setMaximumSize(new Dimension(0x7fffffff, 21));
        txtChnFilename.setMinimumSize(new Dimension(150, 21));
        txtChnFilename.setPreferredSize(new Dimension(9999, 21));
        btnChnLoad.setEnabled(false);
        btnChnLoad.setText("Load");
        btnChnBrowse.setToolTipText("");
        btnChnBrowse.setText("Browse");
        jLabel9.setText("Qualification requirement:");
        mslQualify.setMinimumSize(new Dimension(200, 30));
        mslQualify.setAligned(1);
        lblPlayersRemaining.setText("Number of players remaining");
        jLabel18.setText("Count:");
        txtPlayerCount.setEnabled(false);
        txtPlayerCount.setPreferredSize(new Dimension(21, 21));
        txtPlayerCount.setDisabledTextColor(Color.black);
        txtPlayerCount.setText("txtPlayerCount");
        rbnPointMatch.setSelected(true);
        rbnPointMatch.setText("Points for match");
        rbnPointMap.setText("Points for map");
        jLabel20.setText("Internal group ranking:");
        rbnEqual.setSelected(true);
        rbnEqual.setText("Equal groups");
        jLabel21.setText("Seeding system:");
        rbnDivisions.setToolTipText("");
        rbnDivisions.setText("Division system");
        spTeam.getViewport().add(tblTeam);
        titledBorder5 = new TitledBorder("");
        titledBorder6 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Cup");
        border1 = BorderFactory.createLineBorder(Color.black, 2);
        titledBorder8 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Groups");
        titledBorder7 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "League");
        titledBorder9 = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Contestants signup");
        teamBorder = new TitledBorder(BorderFactory.createEtchedBorder(Color.white, new Color(148, 145, 140)), "Team members - (no team selected):");
        setIconImage(CompoMaster.getIcon());
        String title = "CompoMaster";
        if(CompoMaster.dataFileName != null)
            title = String.valueOf(title) + String.valueOf(" - ".concat(String.valueOf(String.valueOf(CompoMaster.dataFileName))));
        setTitle(title);
        getContentPane().setLayout(borderLayout3);
        titledBorder2 = new TitledBorder("Cup type");
        titledBorder3 = new TitledBorder("Group/league play settings:");
        titledBorder4 = new TitledBorder("DeathMatch options:");
        titledBorder1 = new TitledBorder("");
        optionsPanel.setLayout(borderLayout1);
        pnlGroups.setLayout(gridBagLayout4);
        launchButton.setActionCommand("Launch compo");
        launchButton.setText("Launch compo");
        launchButton.addActionListener(buttonListener);
        pnlTourney.setLayout(borderLayout4);
        pnlCup.setBorder(titledBorder6);
        mslElimination.setValues(new int[] {
            512, 256, 128, 64, 32, 16, 8, 4, 2
        });
        lblNumFinals.setEnabled(true);
        lblNumFinals.setToolTipText("");
        lblNumFinals.setText("Extra finals:");
        sldNumGroups.setMaximum(32);
        sldNumGroups.setMinimum(1);
        sldNumGroups.setMinorTickSpacing(1);
        sldNumGroups.setValue(1);
        sldNumGroups.setSnapToTicks(true);
        sldNumGroups.setPaintTicks(true);
        txtNumGroups.setMinimumSize(new Dimension(20, 21));
        txtNumGroups.setPreferredSize(new Dimension(20, 21));
        txtNumGroups.setText("1");
        pnlGroups.setBorder(titledBorder8);
        pnlGroups.setPreferredSize(new Dimension(300, 94));
        pnlLeague.setLayout(borderLayout6);
        lblPlayers.setToolTipText("");
        lblPlayers.setText("With N number of player signed  up, this results in:");
        lblGroups.setText("4 groups of 3 and 4 groups of 3");
        btnBrowse.setText("Browse");
        btnBrowse.addActionListener(buttonListener);
        playersPanelExt.setBorder(BorderFactory.createRaisedBevelBorder());
        playersPanelExt.setLayout(xYLayout8);
        playersPanelExt.setVisible(true);
        jLabel14.setText("(E.g. \"1-3\" imports players at 1st  to 3rd position from each group)");
        jLabel13.setText("jLabel13");
        jLabel12.setText("Rank range of players to import (1-n):");
        jLabel11.setText("External CM-file:");
        playersPanelReal.setLayout(xYLayout9);
        playersPanelReal.setVisible(true);
        txtNumPlayers.setBorder(BorderFactory.createLineBorder(Color.black));
        txtNumPlayers.setEditable(false);
        playersPanel.setLayout(xYLayout4);
        jLabel6.setText("Total:");
        xYLayout8.setHeight(127);
        xYLayout8.setWidth(395);
        jLabel4.setText("Signed up players:");
        txtName.setFont(MatchList.getMainFont());
        txtName.setNextFocusableComponent(txtRank);
        btnRankDown.setText("D");
        btnRankUp.setMaximumSize(new Dimension(20, 27));
        btnRankUp.setMnemonic('0');
        btnRankUp.setText("U");
        pnlPlayers.setLayout(gridBagLayout6);
        pnlContestants.setLayout(gridBagLayout8);
        txtNameMember.setMinimumSize(new Dimension(30, 21));
        txtNameMember.setPreferredSize(new Dimension(30, 21));
        txtNameMember.setEnabled(false);
        spTeam.setPreferredSize(new Dimension(150, 110));
        pnlTeamMembers.setBorder(teamBorder);
        pnlPlayers.setBorder(titledBorder9);
        jLabel5.setText("Name:");
        rbnTeams.setText("Teams");
        rbnIndividuals.setSelected(true);
        rbnIndividuals.setText("Individuals");
        pnlContTop.setLayout(borderLayout8);
        chkQ3Colors.setHorizontalAlignment(0);
        chkQ3Colors.setSelected(true);
        chkQ3Colors.setText("Enable parsing of Quake 3-style color codes");
        pnlContestants.add(pnlPlayers, new GridBagConstraints(0, 1, 1, 1, 1.0D, 0.69999999999999996D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        btnDelPlayer.setEnabled(false);
        btnDelPlayer.setText("Delete");
        jLabel2.setText("Rank:");
        txtRank.setMinimumSize(new Dimension(30, 21));
        txtRank.setPreferredSize(new Dimension(30, 21));
        txtRank.setToolTipText("");
        txtRank.setText("N/A");
        jLabel1.setText("Name:");
        btnClearPlayer.setText("Clear");
        pnlTeamMembers.setLayout(gridBagLayout7);
        btnClearMember.setToolTipText("");
        btnClearMember.setText("Clear");
        btnClearMember.addActionListener(buttonListener);
        btnDelMember.setText("Delete");
        btnDelMember.addActionListener(buttonListener);
        pnlContestants.add(pnlTeamMembers, new GridBagConstraints(0, 2, 1, 1, 1.0D, 0.29999999999999999D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        if(!data.getTeamMode())
            pnlContestants.remove(pnlTeamMembers);
        pnlLeague.add(pnlLg2, "Center");
        pnlLg2.setBorder(titledBorder7);
        pnlLg2.setLayout(gridBagLayout5);
        pnlLg2.add(txtLgVictory, new GridBagConstraints(1, 4, 1, 1, 0.5D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(txtLgDraw, new GridBagConstraints(1, 5, 2, 1, 0.5D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(jLabel3, new GridBagConstraints(0, 4, 1, 1, 0.5D, 0.0D, 13, 0, new Insets(0, 0, 0, 20), 0, 0));
        pnlLg2.add(lblLgDraw, new GridBagConstraints(0, 5, 1, 1, 0.5D, 0.0D, 13, 0, new Insets(0, 0, 0, 20), 0, 0));
        pnlLg2.add(chkAllowDraws, new GridBagConstraints(0, 6, 2, 1, 0.0D, 1.0D, 11, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(rbnPointMatch, new GridBagConstraints(0, 3, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(rbnPointMap, new GridBagConstraints(1, 3, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(jLabel21, new GridBagConstraints(0, 0, 2, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(rbnDivisions, new GridBagConstraints(1, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(rbnEqual, new GridBagConstraints(0, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlLg2.add(jLabel20, new GridBagConstraints(0, 2, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        tourneyTypeGroup.add(rbnCup);
        tourneyTypeGroup.add(rbnFFA);
        tourneyTypeGroup.add(rbnLeague);
        txtLgVictory.setMinimumSize(new Dimension(20, 21));
        txtLgVictory.setPreferredSize(new Dimension(21, 21));
        txtLgVictory.setToolTipText("");
        txtLgVictory.setText("2");
        txtLgDraw.setText("1");
        txtLgDraw.setMinimumSize(new Dimension(20, 21));
        txtLgDraw.setPreferredSize(new Dimension(21, 21));
        txtLgDraw.setToolTipText("");
        jLabel3.setText("Points awarded for victory:");
        lblLgDraw.setText("Points awarded for draws:");
        chkAllowDraws.setText("Enable draws");
        chkAllowDraws.setSelected(true);
        jLabel10.setText("Tournament name:");
        rbnLeague.addActionListener(buttonListener);
        rbnLeague.setText("League");
        rbnFFA.addActionListener(buttonListener);
        rbnFFA.setText("FFA");
        rbnCup.addActionListener(buttonListener);
        rbnCup.setToolTipText("");
        rbnCup.setSelected(true);
        rbnCup.setText("Cup");
        pnlTourney1.setLayout(borderLayout5);
        txtCompoName.setMinimumSize(new Dimension(100, 21));
        txtCompoName.setPreferredSize(new Dimension(99, 21));
        pnlTourney11.setLayout(borderLayout7);
        borderLayout7.setHgap(5);
        borderLayout7.setVgap(5);
        pnlGroups.add(lblAdvancingPlayers, new GridBagConstraints(2, 3, 0, 0, 0.0D, 0.0D, 17, 0, new Insets(-22, -6, 0, 0), -96, -17));
        pnlGroups.add(sldNumGroups, new GridBagConstraints(0, 0, 1, 1, 1.0D, 0.0D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        pnlGroups.add(txtNumGroups, new GridBagConstraints(1, 0, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 5, 0, 0), 0, 0));
        pnlGroups.add(lblPlayers, new GridBagConstraints(0, 1, 2, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlGroups.add(lblGroups, new GridBagConstraints(0, 2, 2, 1, 0.0D, 1.0D, 11, 0, new Insets(0, 0, 0, 0), 0, 0));
        mslElimination.addArea(new Color(128, 128, 255));
        mslElimination.addArea(Color.pink);
        mslElimination.setSelectedValue(0, 8);
        mslElimination.addChangeListener(buttonListener);
        pnlCup.setLayout(gridBagLayout3);
        pnlCup.add(mslElimination, new GridBagConstraints(0, 1, 2, 1, 1.0D, 0.0D, 10, 2, new Insets(0, 0, 0, 0), 0, 0));
        pnlCup.add(pnlFiller1, new GridBagConstraints(1, 4, 2, 1, 1.0D, 1.0D, 11, 1, new Insets(0, 0, 0, 0), 0, 0));
        pnlCup.add(mslElimination.getDescriptor(0, "Single elimination"), new GridBagConstraints(0, 0, 1, 1, 1.0D, 0.0D, 13, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlCup.add(mslElimination.getDescriptor(1, "Double elimination"), new GridBagConstraints(1, 0, 1, 1, 1.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        btnAddMap.setPreferredSize(new Dimension(125, 27));
        btnAddMap.setText("Add");
        btnDeleteMaps.setMaximumSize(new Dimension(125, 27));
        btnDeleteMaps.setMinimumSize(new Dimension(125, 27));
        btnDeleteMaps.setPreferredSize(new Dimension(125, 27));
        btnDeleteMaps.setText("Delete selected");
        pnlMatch2A.setLayout(gridBagMapPanel);
        xYLayout7.setWidth(221);
        xYLayout7.setHeight(186);
        jLabel19.setText("Number of groups:");
        theMenu = new Menu(data, false);
        rbtMatch.setSelected(true);
        rbtMatch.setText("Match");
        rbtRound.setText("Round");
        jLabel8.setText("Level of score detail: (increasing from left to right)");
        pnlMatch1.setLayout(gridBagLayout1);
        rbtMap.setText("Map");
        lblLodDesc.setText("-");
        pnlMatch1.setToolTipText("");
        btnKeepMaps.setPreferredSize(new Dimension(125, 27));
        btnKeepMaps.setText("Keep selected");
        pnlMatch2.setLayout(borderLayout2);
        btnDeleteMaps.setText("Delete selected");
        btnAddMap.addActionListener(buttonListener);
        btnAddMap.setText("Add");
        pnlMatch2B.setLayout(gridBagLayout2);
        jLabel15.setText("Predefined:");
        jLabel16.setToolTipText("");
        jLabel16.setText("Long:");
        txtRoundShort1.setMaximumSize(new Dimension(25, 21));
        txtRoundShort1.setMinimumSize(new Dimension(25, 21));
        txtRoundShort1.setPreferredSize(new Dimension(25, 21));
        txtRoundLong2.setMaximumSize(new Dimension(100, 0x7fffffff));
        txtRoundLong2.setPreferredSize(new Dimension(50, 21));
        txtRoundShort2.setMaximumSize(new Dimension(25, 21));
        txtRoundShort2.setMinimumSize(new Dimension(25, 21));
        txtRoundShort2.setPreferredSize(new Dimension(25, 21));
        txtRoundLong1.setMaximumSize(new Dimension(100, 0x7fffffff));
        txtRoundLong1.setPreferredSize(new Dimension(50, 21));
        txtRoundShort1.addKeyListener(new TextFieldLimiter(2, false));
        txtRoundShort2.addKeyListener(new TextFieldLimiter(2, false));
        txtRoundLong1.addKeyListener(new TextFieldLimiter(30, false));
        txtRoundLong2.addKeyListener(new TextFieldLimiter(30, false));
        pnlMatch2B.setBorder(titledBorder5);
        titledBorder5.setTitle("Round names:");
        lblDummy1.setToolTipText("");
        jLabel17.setToolTipText("");
        jLabel17.setText("Short:");
        getContentPane().add(theMenu, "North");
        getContentPane().add(jTabbedPane1, "Center");
        pnlMatch2A.add(txtMap, new GridBagConstraints(0, 0, 1, 1, 1.0D, 0.0D, 17, 2, new Insets(0, 0, 0, 0), 100, 0));
        pnlMatch2A.add(btnAddMap, new GridBagConstraints(0, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(3, 0, 3, 0), 5, 0));
        pnlMatch2A.add(btnKeepMaps, new GridBagConstraints(0, 5, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(3, 0, 3, 0), 5, 0));
        pnlMatch2A.add(btnDeleteMaps, new GridBagConstraints(0, 3, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(2, 0, 4, 0), 5, 0));
        pnlMatch2A.add(spMapList1, new GridBagConstraints(0, 2, 1, 1, 1.0D, 1.0D, 13, 1, new Insets(0, 0, 3, 0), 0, 0));
        pnlMatch2A.setBorder(BorderFactory.createTitledBorder("Allowed maps:"));
        pnlMatch1.add(jLabel8, new GridBagConstraints(1, 0, 3, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 5, 0, 5), 25, 0));
        pnlMatch1.add(rbtRound, new GridBagConstraints(3, 1, 1, 1, 1.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 50, 0));
        pnlMatch1.add(rbtMatch, new GridBagConstraints(1, 1, 1, 1, 1.0D, 0.0D, 13, 0, new Insets(0, 0, 0, 0), 50, 0));
        pnlMatch1.add(rbtMap, new GridBagConstraints(2, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 50, 0));
        pnlMatch1.add(lblLodDesc, new GridBagConstraints(1, 2, 3, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        optionsPanel.add(pnlMatch2, "Center");
        optionsPanel.add(pnlMatch1, "North");
        pnlMatch2B.add(jLabel15, new GridBagConstraints(0, 0, 2, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlMatch2B.add(cmbPreDefRounds, new GridBagConstraints(0, 1, 2, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 5, 0), 0, 0));
        pnlMatch2B.add(jLabel16, new GridBagConstraints(0, 2, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlMatch2B.add(txtRoundLong1, new GridBagConstraints(0, 3, -1, 1, 1.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 100, 0));
        pnlMatch2B.add(txtRoundShort1, new GridBagConstraints(1, 3, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlMatch2B.add(txtRoundLong2, new GridBagConstraints(0, 4, -1, 1, 1.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 100, 0));
        pnlMatch2B.add(txtRoundShort2, new GridBagConstraints(1, 4, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlMatch2B.add(lblDummy1, new GridBagConstraints(0, 5, 1, 1, 0.0D, 1.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlMatch2B.add(jLabel17, new GridBagConstraints(1, 2, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(pnlLaunch, "South");
        jTabbedPane1.add(pnlTourney, "Tournament");
        pnlTourney.add(pnlTourney1, "North");
        pnlTourney1.add(pnlTourney11, "North");
        pnlTourney11.add(jLabel10, "West");
        pnlTourney11.add(txtCompoName, "Center");
        pnlTourney1.add(pnlTourney12, "Center");
        pnlTourney12.add(rbnCup, null);
        pnlTourney12.add(rbnLeague, null);
        pnlTourney12.add(rbnFFA, null);
        jTabbedPane1.add(optionsPanel, "Matches");
        lstMaps.setBorder(BorderFactory.createLineBorder(Color.black));
        pnlMatch2.add(pnlMatch2A, "Center");
        pnlMatch2.add(pnlMatch2B, "East");
        matchDetailGroup.add(rbtMatch);
        matchDetailGroup.add(rbtRound);
        matchDetailGroup.add(rbtMap);
        pnlLaunch.add(launchButton, null);
        pnlTourney.add(pnlCup, "Center");
        TableColumnModel tcm = tblPlayers.getColumnModel();
        tcm.setColumnSelectionAllowed(false);
        tcm.getColumn(0).setResizable(false);
        tcm.getColumn(1).setResizable(false);
        tcm.getColumn(1).setMaxWidth(50);
        tblTeam.setRowSelectionAllowed(true);
        PFocusAdapter focusListener = new PFocusAdapter();
        TableListener tableListener = new TableListener();
        tblPlayers.getSelectionModel().addListSelectionListener(tableListener);
        tblTeam.getSelectionModel().addListSelectionListener(tableListener);
        btnRankUp.addActionListener(buttonListener);
        btnRankDown.addActionListener(buttonListener);
        btnDelPlayer.addActionListener(buttonListener);
        btnClearPlayer.addActionListener(buttonListener);
        btnDelMember.addActionListener(buttonListener);
        btnClearMember.addActionListener(buttonListener);
        txtName.addFocusListener(focusListener);
        txtName.addKeyListener(keyListener);
        txtRank.addKeyListener(keyListener);
        txtRank.addFocusListener(focusListener);
        rbnIndividuals.addActionListener(buttonListener);
        rbnTeams.addActionListener(buttonListener);
        btnAddMap.addActionListener(buttonListener);
        btnDeleteMaps.addActionListener(buttonListener);
        btnKeepMaps.addActionListener(buttonListener);
        txtRoundShort1.addKeyListener(keyListener);
        txtRoundShort2.addKeyListener(keyListener);
        txtRoundLong1.addKeyListener(keyListener);
        txtRoundLong2.addKeyListener(keyListener);
        txtMap.addKeyListener(keyListener);
        rbtMatch.addActionListener(buttonListener);
        rbtMap.addActionListener(buttonListener);
        rbtRound.addActionListener(buttonListener);
        cmbPreDefRounds.addActionListener(buttonListener);
        cellTF.addFocusListener(focusListener);
        sldNumGroups.addChangeListener(buttonListener);
        txtNumGroups.addActionListener(buttonListener);
        txtNumGroups.addFocusListener(focusListener);
        chkAllowDraws.addActionListener(buttonListener);
        rbnPointMatch.addActionListener(buttonListener);
        rbnPointMap.addActionListener(buttonListener);
        btnChnBrowse.addActionListener(buttonListener);
        btnChnLoad.addActionListener(buttonListener);
        txtChnFilename.addKeyListener(keyListener);
        rbnStandalone.addActionListener(buttonListener);
        rbnQualify.addActionListener(buttonListener);
        mslQualify.addChangeListener(buttonListener);
        if(data.matchList instanceof GroupStructure)
            jTabbedPane1.add(pnlGroups, "Groups");
        if(!data.getTeamMode());
        TableColumn rankCol = tblPlayers.getColumn("Ranking:");
        rankCol.setPreferredWidth(50);
        rankCol.setResizable(false);
        DefaultCellEditor ced = new DefaultCellEditor(cellTF);
        ced.setClickCountToStart(1);
        rankCol.setCellEditor(ced);
        TableColumn playerCol = tblPlayers.getColumn("Contestant:");
        playerCol.setPreferredWidth(200);
        playerCol.setResizable(false);
        playerCol.setMinWidth(200);
        lstMaps.setListData(mapList);
        for(int i = 0; i < preDefRoundName.length; i++)
            cmbPreDefRounds.addItem(preDefRoundName[i]);

        chainUpdateDesc();
        lblAdvancingPlayers.setText("Position from top:");
        lblAdvancingPlayers.setVisible(false);
        compoLoaded();
        pack();
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension thisSize = getSize();
        setLocation((screen.width - thisSize.width) / 2, (screen.height - thisSize.height) / 2);
        playersPanel.add(playersPanelReal, new XYConstraints(12, 14, 400, 429));
        playersPanelReal.add(jLabel6, new XYConstraints(12, 360, 34, 16));
        playersPanelReal.add(txtNumPlayers, new XYConstraints(55, 359, 62, 18));
        playersPanelReal.add(jLabel4, new XYConstraints(10, 145, 116, 18));
        playersPanel.add(playersPanelExt, new XYConstraints(196, 203, 432, 153));
        playersPanelExt.add(jLabel11, new XYConstraints(12, 23, 90, 24));
        playersPanelExt.add(txtExtFileName, new XYConstraints(108, 23, 183, 28));
        playersPanelExt.add(jLabel13, new XYConstraints(-10, 109, 16, 4));
        playersPanelExt.add(jLabel12, new XYConstraints(12, 61, 213, 28));
        playersPanelExt.add(btnBrowse, new XYConstraints(300, 25, 83, 24));
        playersPanelExt.add(txtRanks, new XYConstraints(228, 63, 62, 22));
        playersPanelExt.add(jLabel14, new XYConstraints(8, 89, 367, 22));
        txtName.requestDefaultFocus();
        pnlPlayers.add(txtName, new GridBagConstraints(0, 1, 1, 1, 1.0D, 0.0D, 17, 2, new Insets(0, 0, 0, 0), 0, 0));
        pnlPlayers.add(jLabel1, new GridBagConstraints(0, 0, 1, 1, 1.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlPlayers.add(jLabel2, new GridBagConstraints(1, 0, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlPlayers.add(pnlRank, new GridBagConstraints(1, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlRank.add(btnRankUp, null);
        pnlRank.add(txtRank, null);
        pnlRank.add(btnRankDown, null);
        pnlPlayers.add(pnlButtons1, new GridBagConstraints(2, 1, 1, 1, 0.0D, 0.0D, 13, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlButtons1.add(btnDelPlayer, null);
        pnlButtons1.add(btnClearPlayer, null);
        pnlPlayers.add(spPlayers, new GridBagConstraints(0, 2, 3, 1, 0.0D, 0.5D, 13, 1, new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(jLabel18, null);
        jPanel1.add(txtPlayerCount, null);
        pnlPlayers.add(jPanel1, new GridBagConstraints(2, 3, 1, 1, 0.0D, 0.0D, 13, 0, new Insets(0, 0, 0, 0), 0, 0));
        tblTeam.setPreferredScrollableViewportSize(new Dimension(150, 110));
        tblPlayers.setPreferredScrollableViewportSize(new Dimension(200, 150));
        pnlTeamMembers.add(pnlButtons2, new GridBagConstraints(1, 0, 1, 2, 0.0D, 0.0D, 13, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlButtons2.add(btnDelMember, null);
        pnlButtons2.add(btnClearMember, null);
        jTabbedPane1.add(pnlContestants, "Contestants");
        pnlTeamMembers.add(txtNameMember, new GridBagConstraints(0, 1, 1, 1, 0.80000000000000004D, 0.0D, 10, 2, new Insets(0, 0, 0, 0), 0, 0));
        pnlTeamMembers.add(spTeam, new GridBagConstraints(0, 2, 2, 2, 0.0D, 1.0D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        pnlTeamMembers.add(jLabel5, new GridBagConstraints(0, 0, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlContestants.add(pnlContTop, new GridBagConstraints(0, 0, 1, 1, 0.0D, 0.0D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        txtNameMember.addKeyListener(keyListener);
        bgContestants.add(rbnTeams);
        bgContestants.add(rbnIndividuals);
        pnlContTop.add(pnlRadioButtons, "Center");
        pnlRadioButtons.add(rbnTeams, null);
        pnlRadioButtons.add(rbnIndividuals, null);
        pnlContTop.add(chkQ3Colors, "South");
        jTabbedPane1.add(pnlChaining, "Chaining");
        pnlChnIn.add(rbnStandalone, new GridBagConstraints(0, 0, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlChnIn.add(rbnQualify, new GridBagConstraints(0, 1, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlChaining.add(pnlChnOut, "South");
        pnlChaining.add(pnlChnIn, "Center");
        pnlChnOut.add(jTextArea1, "Center");
        bgChain.add(rbnQualify);
        bgChain.add(rbnStandalone);
        pnlChnIn.add(pnlQualify, new GridBagConstraints(0, 2, 1, 1, 1.0D, 1.0D, 11, 2, new Insets(0, 0, 0, 0), 0, 0));
        if(rbnStandalone.isSelected())
            pnlChnIn.remove(pnlQualify);
        pnlQualify.add(txtChnFilename, new GridBagConstraints(0, 1, 1, 1, 1.0D, 0.0D, 10, 2, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(btnChnLoad, new GridBagConstraints(1, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(btnChnBrowse, new GridBagConstraints(2, 1, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlChnIn.add(pnlDummy, new GridBagConstraints(0, 3, 1, 1, 1.0D, 1.0D, 11, 1, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(jLabel9, new GridBagConstraints(0, 2, 3, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(lblQualify, new GridBagConstraints(0, 4, 3, 1, 0.0D, 1.0D, 11, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(jLabel7, new GridBagConstraints(0, 0, 3, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlQualify.add(mslQualify, new GridBagConstraints(0, 3, 3, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlCup.add(lblPlayersRemaining, new GridBagConstraints(0, 2, 2, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        pnlCup.add(lblNumFinals, new GridBagConstraints(0, 3, 1, 1, 0.5D, 0.0D, 13, 0, new Insets(15, 0, 0, 5), 0, 0));
        pnlCup.add(cmbNumFinals, new GridBagConstraints(1, 3, 1, 1, 0.5D, 0.0D, 17, 0, new Insets(15, 5, 0, 0), 0, 0));
        txtPlayerCount.setHorizontalAlignment(0);
        bgPointSystem.add(rbnPointMatch);
        bgPointSystem.add(rbnPointMap);
        bgSeedingSystem.add(rbnDivisions);
        bgSeedingSystem.add(rbnEqual);
    }

    public void compoLoaded()
    {
        data = CompoMaster.getDataInstance();
        txtCompoName.setText(data.getCompoName());
        if(data.matchList instanceof CupStructure)
        {
            rbnCup.setSelected(true);
            mslElimination.setSelectedValue(0, data.cupSliderVal);
        }
        if(data.matchList instanceof LeagueGroups)
        {
            LeagueGroups ml = (LeagueGroups)data.matchList;
            rbnLeague.setSelected(true);
            sldNumGroups.setValue(ml.getNumGroups());
            txtLgDraw.setText("".concat(String.valueOf(String.valueOf(ml.getPointsDraw()))));
            txtLgVictory.setText("".concat(String.valueOf(String.valueOf(ml.getPointsVictory()))));
            chkAllowDraws.setSelected(ml.getDrawsEnabled());
            txtLgDraw.setEnabled(ml.getDrawsEnabled());
            if(ml.isDivisionSystem())
                rbnDivisions.setSelected(true);
            else
                rbnEqual.setSelected(true);
            boolean pfm = ml.isPointsForMatch();
            rbnPointMatch.setSelected(pfm);
            rbnPointMap.setSelected(!pfm);
            toggleLeaguePointSystem(pfm);
            sldNumGroups.setValue(ml.getNumGroups());
        }
        if(data.matchList instanceof FFAGroups)
        {
            rbnFFA.setSelected(true);
            GroupStructure ml = (GroupStructure)data.matchList;
            sldNumGroups.setValue(ml.getNumGroups());
        }
        if(data.getTeamMode())
            rbnTeams.setSelected(true);
        else
            rbnIndividuals.setSelected(true);
        chkQ3Colors.setSelected(data.enableQ3Colors);
        setPlayerMode();
        if(data.doRounds)
            rbtRound.setSelected(true);
        else
        if(data.doMaps)
            rbtMap.setSelected(true);
        else
            rbtMatch.setSelected(true);
        txtRoundLong1.setText(data.getRoundNameLong(0));
        txtRoundLong2.setText(data.getRoundNameLong(1));
        txtRoundShort1.setText(data.getRoundNameShort(0));
        txtRoundShort2.setText(data.getRoundNameShort(1));
        setScoreMode();
        dataToGui();
    }

    private void setFDEFinals(boolean FDE)
    {
        String s[];
        if(FDE)
            s = deFinales;
        else
            s = seFinales;
        int sel = cmbNumFinals.getSelectedIndex();
        cmbNumFinals.removeAllItems();
        for(int i = 0; i < s.length; i++)
            cmbNumFinals.addItem(s[i]);

        if(s.length > sel)
            cmbNumFinals.setSelectedIndex(sel);
    }

    protected void processWindowEvent(WindowEvent e)
    {
        super.processWindowEvent(e);
        if(e.getID() == 201)
            theMenu.exit();
    }

    public void guiToData()
    {
        setTournamentData();
        data.enableQ3Colors = chkQ3Colors.isSelected();
        if(data.doRounds)
        {
            data.setRoundName(0, txtRoundShort1.getText(), txtRoundLong1.getText());
            data.setRoundName(1, txtRoundShort2.getText(), txtRoundLong2.getText());
        }
        int numSlid = mslQualify.getNumSliders();
        if(numSlid == 2)
        {
            data.inRankLow = mslQualify.getSelectedValue(1);
            data.inRankHigh = mslQualify.getSelectedValue(0) + 1;
        } else
        if(numSlid == 1)
            data.inRankLow = mslQualify.getSelectedValue(0);
        if(impData != null)
        {
            impData.outRankLow = data.inRankLow;
            impData.outRankHigh = data.inRankHigh;
        }
    }

    public void dataToGui()
    {
        updateTourneyTab();
        data.sortPlayers(0);
        txtPlayerCount.setText("".concat(String.valueOf(String.valueOf(data.getNumPlayers()))));
        tableModel.update();
        mapList = new String[data.getNumMaps()];
        String s;
        for(int i = 0; (s = data.getMapName(i)) != null; i++)
            mapList[i] = s;

        lstMaps.setListData(mapList);
        txtNumPlayers.setText(String.valueOf(data.getNumPlayers()));
        if(data.matchList instanceof GroupStructure)
            updateGroupTab(0);
        updateGroupTab(1);
        String extName = data.getInDataFilename();
        if(extName != null && !txtChnFilename.getText().equals(extName))
        {
            txtChnFilename.setText(extName);
            chainFileChanged();
            if(impData != null)
                if(mslQualify.getNumSliders() == 2)
                {
                    mslQualify.setSelectedValue(0, data.inRankHigh);
                    mslQualify.setSelectedValue(1, data.inRankLow);
                } else
                {
                    mslQualify.setSelectedValue(0, data.inRankLow);
                }
        }
    }

    public void paint(Graphics g)
    {
        super.paint(g);
        if(firstTime)
        {
            playersPanelExt.setVisible(false);
            firstTime = false;
        }
    }

    private void enterButtonPressed()
    {
        storePlayerChanges();
        clearPlayerInput();
        dataToGui();
    }

    private void storeMember()
    {
        if(memberEditRange.length == 1)
        {
            selectedTeam.setMember(memberEditRange[0], txtNameMember.getText());
            memberEditRange = null;
        } else
        {
            selectedTeam.addMember(txtNameMember.getText());
        }
        txtNameMember.setText("");
        txtNameMember.grabFocus();
        teamMemberModel.update();
    }

    private void delMember()
    {
        int rows[] = tblTeam.getSelectedRows();
        int j = 0;
        for(int i = 0; i < rows.length; i++)
            selectedTeam.removeMember(rows[i - j++]);

        data.isChanged = true;
        teamMemberModel.update();
    }

    private void storePlayerChanges()
    {
        int numSelected = 0;
        if(playerEditRange != null)
            numSelected = playerEditRange.length;
        int rank = readRankField();
        if(numSelected < 2)
        {
            String name = txtName.getText().trim();
            if(name.length() < 1)
            {
                txtName.grabFocus();
                return;
            }
            if(numSelected == 0)
            {
                for(int i = 0; i < data.getNumPlayers(); i++)
                    if(data.getPlayer(i).getName().equals(name))
                    {
                        JOptionPane.showMessageDialog(CompoMaster.activeFrame, String.valueOf(String.valueOf((new StringBuffer("Player with name ")).append(name).append(" already exists"))), "Input error", 0);
                        txtName.grabFocus();
                        return;
                    }

                Player pr = new Team(txtName.getText(), "");
                pr.setRank(rank);
                data.addPlayer(pr);
            } else
            {
                int idx = playerEditRange[0];
                Player p = data.getPlayer(idx);
                if(p instanceof RealPlayer)
                {
                    RealPlayer pr = (RealPlayer)p;
                    pr.setName(name);
                }
                p.setRank(rank);
            }
        } else
        {
            int idx[] = playerEditRange;
            for(int i = 0; i < idx.length; i++)
            {
                Player p = data.getPlayer(idx[i]);
                p.setRank(rank);
            }

        }
        data.isChanged = true;
    }

    private void clearPlayerInput()
    {
        playerEditRange = new int[0];
        tblPlayers.clearSelection();
        txtName.grabFocus();
        txtName.setText("");
        txtRank.setText("N/A");
    }

    private void clearMemberInput()
    {
        memberEditRange = new int[0];
        tblTeam.clearSelection();
        txtNameMember.setText("");
        txtNameMember.grabFocus();
    }

    public void btnAddMapPressed()
    {
        String mapname = txtMap.getText().trim();
        if(mapname.length() < 1)
        {
            txtMap.grabFocus();
            return;
        }
        for(int i = 0; i < data.getNumMaps(); i++)
            if(data.getMapName(i).equals(mapname))
            {
                JOptionPane.showMessageDialog(CompoMaster.activeFrame, String.valueOf(String.valueOf((new StringBuffer("Map with name ")).append(mapname).append(" already exists"))), "Error", 0);
                txtMap.grabFocus();
                txtMap.setSelectionStart(0);
                return;
            }

        data.addMap(mapname);
        txtMap.setText("");
        txtMap.grabFocus();
        dataToGui();
    }

    public void btnKeepMaps()
    {
        for(int i = data.getNumMaps() - 1; i >= 0; i--)
            if(!lstMaps.isSelectedIndex(i))
                data.removeMap(i);

        data.isChanged = true;
        dataToGui();
    }

    public void btnDeleteMaps()
    {
        for(int i = data.getNumMaps() - 1; i >= 0; i--)
            if(lstMaps.isSelectedIndex(i))
                data.removeMap(i);

        data.isChanged = true;
        dataToGui();
    }

    private int readRankField()
    {
        try
        {
            int i = Integer.parseInt(txtRank.getText());
            return i;
        }
        catch(NumberFormatException e)
        {
            char c = '\u0101';
            return c;
        }
    }

    private void validateRankField()
    {
        int rank = readRankField();
        if(rank == 0)
            txtRank.setText("N/A");
        if(rank < 0)
            txtRank.setText("256");
        if(rank == Data.MAXRANK)
            txtRank.setText("N/A");
        if(rank > Data.MAXRANK)
            txtRank.setText("1");
    }

    private void rankButtonPressed(int val)
    {
        int newRank = readRankField() + val;
        txtRank.setText("".concat(String.valueOf(String.valueOf(newRank))));
        validateRankField();
        playerEditRange = tblPlayers.getSelectedRows();
    }

    private void deleteButtonPressed()
    {
        int rows[] = tblPlayers.getSelectedRows();
        int j = 0;
        for(int i = 0; i < rows.length; i++)
            data.removePlayer(rows[i - j++]);

        data.isChanged = true;
        dataToGui();
    }

    private void setPlayerMode()
    {
        if(rbnIndividuals.isSelected())
        {
            pnlContestants.remove(pnlTeamMembers);
            data.setTeamMode(false);
            pnlContestants.validate();
        } else
        {
            pnlContestants.add(pnlTeamMembers, new GridBagConstraints(0, 2, 1, 1, 1.0D, 0.29999999999999999D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
            data.setTeamMode(true);
            pnlContestants.validate();
        }
    }

    private void setTournamentData()
    {
        int numGroups = sldNumGroups.getValue();
        data.setCompoName(txtCompoName.getText());
        if(rbnCup.isSelected())
        {
            int seRounds = 0;
            int numPl = data.getNumPlayers();
            int start = mslElimination.getSelectedValue(0);
            data.cupSliderVal = start;
            if(start == 2)
                start = 1;
            for(int i = start; i < numPl; i *= 2)
                seRounds++;

            data.setModeFDE(0, cmbNumFinals.getSelectedIndex(), seRounds);
        }
        if(rbnLeague.isSelected())
        {
            data.setModeLeague(chkAllowDraws.isSelected(), Integer.parseInt(txtLgVictory.getText()), Integer.parseInt(txtLgDraw.getText()));
            ((GroupStructure)data.matchList).setNumGroups(numGroups);
            ((LeagueGroups)data.matchList).setPointsForMatch(rbnPointMatch.isSelected());
            ((LeagueGroups)data.matchList).setDivisionSystem(rbnDivisions.isSelected());
        }
        if(rbnFFA.isSelected())
        {
            data.setModeFFA();
            ((GroupStructure)data.matchList).setNumGroups(numGroups);
        }
    }

    private void launchCompo()
    {
        guiToData();
        if(!data.getTeamMode())
            data.teamsToPlayers();
        try
        {
            if(data.doMaps && data.getNumMaps() < 1)
                throw new Exception("You must specify at least one allowed map");
            if(data.getNumPlayers() < 4)
                throw new Exception("Too few players, must be at least 4");
            data.isChanged = true;
            data.matchList.initCompo();
            data.isLaunched = true;
            CompoMaster.mainAdmin();
        }
        catch(Exception e)
        {
            JOptionPane.showMessageDialog(null, "Error: ".concat(String.valueOf(String.valueOf(e.toString()))), "Unable to launch", 0);
        }
    }

    private void updateGroupTab(int source)
    {
        if(source == 1)
            txtNumGroups.setText("".concat(String.valueOf(String.valueOf(sldNumGroups.getValue()))));
        else
            try
            {
                int val = Integer.parseInt(txtNumGroups.getText());
                sldNumGroups.setValue(val);
                if(val > 32)
                    txtNumGroups.setText("".concat(String.valueOf(String.valueOf(val))));
            }
            catch(NumberFormatException numberformatexception) { }
        int numPlayers = data.getNumPlayers();
        double ppg = 0.0D;
        String t = "";
        int numGroups = 1;
        try
        {
            numGroups = Integer.parseInt(txtNumGroups.getText());
        }
        catch(NumberFormatException numberformatexception1) { }
        int maxGroups = (int)Math.ceil((double)numPlayers / 2D);
        if(numGroups < 1)
            numGroups = 1;
        if(numPlayers > 0)
        {
            ppg = (double)numPlayers / (double)numGroups;
            int rest = numPlayers % numGroups;
            if(rest != 0)
                t = String.valueOf(String.valueOf((new StringBuffer("")).append(rest).append(" group(s) of ").append(numPlayers / numGroups + 1).append(" contestants and ").append(numGroups - rest).append(" groups of ").append(numPlayers / numGroups).append(" contestants.")));
            else
                t = String.valueOf(String.valueOf((new StringBuffer("")).append(numGroups).append(" group(s) of ").append((int)ppg).append(" contestants.")));
        }
        lblPlayers.setText(String.valueOf(String.valueOf((new StringBuffer("With ")).append(numPlayers).append(" contestants signed  up, this results in:"))));
        lblGroups.setText(t);
        if(data.matchList instanceof GroupStructure)
            ((GroupStructure)data.matchList).setNumGroups(numGroups);
    }

    private void enablePanel(JPanel pan, boolean state)
    {
        int cnt = pan.getComponentCount();
        for(int i = 0; i < cnt; i++)
        {
            Component c = pan.getComponent(i);
            c.setEnabled(state);
        }

    }

    private void setScoreMode()
    {
        if(rbtMatch.isSelected())
            setScoreMode(0);
        if(rbtMap.isSelected())
            setScoreMode(1);
        if(rbtRound.isSelected())
            setScoreMode(2);
    }

    private void setScoreMode(int level)
    {
        lblLodDesc.setText(mapLodHelp[level]);
        switch(level)
        {
        case 0: // '\0'
            enablePanel(pnlMatch2A, false);
            enablePanel(pnlMatch2B, false);
            data.doMaps = false;
            data.doRounds = false;
            break;

        case 1: // '\001'
            enablePanel(pnlMatch2A, true);
            enablePanel(pnlMatch2B, false);
            data.doMaps = true;
            data.doRounds = false;
            break;

        case 2: // '\002'
            enablePanel(pnlMatch2A, true);
            enablePanel(pnlMatch2B, true);
            data.doMaps = true;
            data.doRounds = true;
            break;
        }
    }

    private void setPreDef(int idx)
    {
        if(idx != 0)
        {
            txtRoundLong1.setText(preDefRoundLong[idx][0]);
            txtRoundLong2.setText(preDefRoundLong[idx][1]);
            txtRoundShort1.setText(preDefRoundShort[idx][0]);
            txtRoundShort2.setText(preDefRoundShort[idx][1]);
        }
    }

    private void showQualifyPanel(boolean show)
    {
        if(show)
            pnlChnIn.add(pnlQualify, new GridBagConstraints(0, 2, 1, 1, 1.0D, 1.0D, 10, 1, new Insets(0, 0, 0, 0), 0, 0));
        else
            pnlChnIn.remove(pnlQualify);
        pnlChnIn.validate();
    }

    private void chainFileChanged()
    {
        btnChnLoad.setEnabled(true);
        mslQualify.removeAreas();
        impData = null;
        chainUpdateDesc();
    }

    private void chainUpdateDesc()
    {
        if(impData == null)
            lblQualify.setText("You must select and load a .CMP file to complete the setup.");
        else
        if(impData.matchList instanceof GroupStructure)
            lblQualify.setText(String.valueOf(String.valueOf((new StringBuffer("Players ranking from ")).append(mslQualify.getSelectedValue(0) + 1).append(" to ").append(mslQualify.getSelectedValue(1)).append(" in each group will qualify"))));
        else
            lblQualify.setText(String.valueOf(String.valueOf((new StringBuffer("The best ")).append(mslQualify.getSelectedValue(0)).append(" players will qualify."))));
    }

    private void chainLoadFile()
    {
        boolean ok = false;
        File f = new File(txtChnFilename.getText());
        impData = Data.load(String.valueOf(CompoMaster.loadedFileDir) + String.valueOf(f.getName()));
        JOptionPane.showMessageDialog(this, String.valueOf(CompoMaster.loadedFileDir) + String.valueOf(f.getName()), "Wrong file type", 0);
        mslQualify.removeAreas();
        if(impData != null)
        {
            data.setInDataFilename(f.getName());
            data.setExtData(impData);
            if(impData.matchList instanceof GroupStructure)
            {
                mslQualify.setValues(0, ((GroupStructure)impData.matchList).getLargestGroupSize(), 1);
                mslQualify.addArea(CHN_OUT);
                mslQualify.addArea(CHN_IN);
                mslQualify.addArea(CHN_OUT);
                mslQualify.setSelectedValue(0, 0);
                mslQualify.setSelectedValue(1, 1);
                ok = true;
            } else
            {
                int len = 1 + ((CupStructure)impData.matchList).getNumRounds();
                int values[] = new int[len];
                int j = 1;
                for(int i = 1; j < len; i *= 2)
                    values[j++] = i;

                mslQualify.setValues(values);
                mslQualify.addArea(CHN_IN);
                mslQualify.addArea(CHN_OUT);
                mslQualify.setSelectedValue(0, 1);
                ok = true;
            }
        } else
        {
            JOptionPane.showMessageDialog(this, "File is not a CompoMaster file.", "Wrong file type", 0);
        }
        if(!ok)
        {
            txtChnFilename.setSelectionStart(0);
            txtChnFilename.setSelectionEnd(txtChnFilename.getText().length());
            txtChnFilename.grabFocus();
        }
        chainUpdateDesc();
        data.inRankHigh = data.inRankLow = 1;
        updateImportedPlayers();
    }

    private void updateImportedPlayers()
    {
        guiToData();
        data.transferExternalPlayers();
        dataToGui();
    }

    private void chainBrowseFile()
    {
        File file = CompoMaster.loadBrowser();
        if(file != null)
        {
            txtChnFilename.setText(file.getName());
            chainLoadFile();
        }
    }

    private void toggleLeaguePointSystem(boolean isMatch)
    {
        txtLgVictory.setEnabled(isMatch);
        txtLgDraw.setEnabled(isMatch);
        chkAllowDraws.setEnabled(isMatch);
    }

    private void updateTourneyTab()
    {
        if(rbnCup.isSelected())
        {
            pnlTourney.remove(pnlCup);
            pnlTourney.remove(pnlGroups);
            pnlTourney.remove(pnlLeague);
            pnlTourney.add(pnlCup, "Center");
            repaint();
        } else
        if(rbnLeague.isSelected())
        {
            pnlTourney.remove(pnlCup);
            pnlTourney.remove(pnlGroups);
            pnlTourney.remove(pnlLeague);
            pnlLeague.add(pnlGroups, "North");
            pnlTourney.add(pnlLeague, "Center");
            repaint();
        } else
        if(rbnFFA.isSelected())
        {
            pnlTourney.remove(pnlCup);
            pnlTourney.remove(pnlGroups);
            pnlTourney.remove(pnlLeague);
            pnlTourney.add(pnlGroups, "Center");
            repaint();
        }
    }

    private static final int MAXGROUPS = 32;
    private static final String N_A = "N/A";
    Color CHN_OUT;
    Color CHN_IN;
    private Data impData;
    String fileName;
    String mapList[];
    String preDefRoundName[] = {
        "(Custom)", "CT/T", "Ax/Ai"
    };
    String preDefRoundLong[][] = {
        {
            "", ""
        }, {
            "Counter-Terrorists", "Terrorists"
        }, {
            "Axis", "Allies"
        }
    };
    String preDefRoundShort[][] = {
        {
            "", ""
        }, {
            "CT", "T"
        }, {
            "Ax", "Ai"
        }
    };
    String mapLodHelp[] = {
        "Match results needs only to be recorded with a total score.", "Match results must be recorded with a total score and scores for each specific map.", "Match results must be recorded with a total score and scores for two rounds per map."
    };
    private Team selectedTeam;
    private boolean firstTime;
    private int playerEditRange[];
    private int memberEditRange[];
    private Data data;
    String seFinales[] = {
        "None", "One (3rd/4th place)"
    };
    String deFinales[] = {
        "None", "One (5th/6th place)", "Two (5th-8th place)"
    };
    JTabbedPane jTabbedPane1;
    JPanel pnlTourney;
    JPanel optionsPanel;
    JPanel pnlGroups;
    JList lstMaps;
    PlayerTableModel tableModel;
    JTable tblPlayers;
    ButtonGroup cupTypeGroup;
    ButtonGroup winModeGroup;
    Menu theMenu;
    JTextField cellTF;
    ButtonListener buttonListener;
    KeyListener keyListener;
    JMenuItem newMenu;
    TitledBorder titledBorder2;
    TitledBorder titledBorder3;
    TitledBorder titledBorder4;
    JPanel pnlMatch2A;
    JButton btnAddMap;
    JButton btnDeleteMaps;
    JTextField txtMap;
    TitledBorder titledBorder1;
    XYLayout xYLayout7;
    JLabel jLabel19;
    JPanel pnlPlayers;
    JLabel lblAdvancingPlayers;
    BorderLayout borderLayout1;
    JPanel pnlMatch1;
    JRadioButton rbtMatch;
    JRadioButton rbtRound;
    JLabel jLabel8;
    GridBagLayout gridBagLayout1;
    GridBagLayout gblTeamMembers;
    JRadioButton rbtMap;
    JLabel lblLodDesc;
    ButtonGroup matchDetailGroup;
    GridBagLayout gridBagMapPanel;
    JButton btnKeepMaps;
    JPanel pnlMatch2;
    BorderLayout borderLayout2;
    JScrollPane spMapList1;
    JPanel pnlMatch2B;
    GridBagLayout gridBagLayout2;
    JLabel jLabel15;
    JComboBox cmbPreDefRounds;
    JLabel jLabel16;
    JTextField txtRoundLong1;
    JTextField txtRoundShort1;
    JTextField txtRoundLong2;
    JTextField txtRoundShort2;
    TitledBorder titledBorder5;
    BorderLayout borderLayout3;
    JLabel lblDummy1;
    JLabel jLabel17;
    JPanel pnlLaunch;
    JButton launchButton;
    BorderLayout borderLayout4;
    JPanel pnlTourney1;
    JPanel pnlCup;
    ButtonGroup tourneyTypeGroup;
    TitledBorder titledBorder6;
    JPanel pnlLeague;
    MultipleSlider mslElimination;
    JLabel lblNumFinals;
    GridBagLayout gridBagLayout3;
    JPanel pnlFiller1;
    JSlider sldNumGroups;
    JTextField txtNumGroups;
    Border border1;
    TitledBorder titledBorder8;
    GridBagLayout gridBagLayout4;
    TitledBorder titledBorder7;
    JPanel pnlLg2;
    BorderLayout borderLayout6;
    JTextField txtLgVictory;
    JTextField txtLgDraw;
    JLabel jLabel3;
    JLabel lblLgDraw;
    JCheckBox chkAllowDraws;
    GridBagLayout gridBagLayout5;
    JPanel pnlTourney11;
    JPanel pnlTourney12;
    JTextField txtCompoName;
    JLabel jLabel10;
    JRadioButton rbnLeague;
    JRadioButton rbnFFA;
    JRadioButton rbnCup;
    BorderLayout borderLayout5;
    BorderLayout borderLayout7;
    JLabel lblPlayers;
    JLabel lblGroups;
    TitledBorder titledBorder9;
    JButton btnBrowse;
    JPanel playersPanelExt;
    JLabel jLabel14;
    JLabel jLabel13;
    JLabel jLabel12;
    JLabel jLabel11;
    JTextField txtRanks;
    JTextField txtExtFileName;
    JPanel playersPanelReal;
    JTextField txtNumPlayers;
    JPanel playersPanel;
    JLabel jLabel6;
    XYLayout xYLayout9;
    XYLayout xYLayout8;
    JLabel jLabel4;
    XYLayout xYLayout4;
    GridBagLayout gridBagLayout6;
    JTextField txtName;
    JButton btnRankDown;
    JButton btnRankUp;
    JPanel pnlContestants;
    JButton btnDelPlayer;
    JScrollPane spPlayers;
    JPanel pnlButtons1;
    JLabel jLabel2;
    JTextField txtRank;
    JLabel jLabel1;
    JPanel pnlRank;
    JButton btnClearPlayer;
    JScrollPane spTeam;
    JTable tblTeam;
    MemberTableModel teamMemberModel;
    JPanel pnlButtons2;
    JPanel pnlTeamMembers;
    GridBagLayout gridBagLayout7;
    JTextField txtNameMember;
    JButton btnClearMember;
    JButton btnDelMember;
    TitledBorder teamBorder;
    GridBagLayout gridBagLayout8;
    JLabel jLabel5;
    JRadioButton rbnTeams;
    JPanel pnlContTop;
    JRadioButton rbnIndividuals;
    ButtonGroup bgContestants;
    JPanel pnlRadioButtons;
    BorderLayout borderLayout8;
    JCheckBox chkQ3Colors;
    JPanel pnlChaining;
    BorderLayout borderLayout9;
    JPanel pnlChnIn;
    JPanel pnlChnOut;
    TitledBorder titledBorder10;
    TitledBorder titledBorder11;
    GridBagLayout gridBagLayout9;
    JRadioButton rbnQualify;
    JRadioButton rbnStandalone;
    JLabel jLabel7;
    JTextArea jTextArea1;
    BorderLayout borderLayout10;
    ButtonGroup bgChain;
    MultipleSlider mslQualify;
    JLabel lblQualify;
    JPanel pnlQualify;
    GridBagLayout gridBagLayout10;
    JTextField txtChnFilename;
    JButton btnChnLoad;
    JButton btnChnBrowse;
    JLabel jLabel9;
    JPanel pnlDummy;
    JLabel lblPlayersRemaining;
    JComboBox cmbNumFinals;
    JLabel jLabel18;
    JTextField txtPlayerCount;
    JPanel jPanel1;
    JRadioButton rbnPointMatch;
    JRadioButton rbnPointMap;
    ButtonGroup bgPointSystem;
    JLabel jLabel20;
    JRadioButton rbnEqual;
    JLabel jLabel21;
    JRadioButton rbnDivisions;
    ButtonGroup bgSeedingSystem;






























}