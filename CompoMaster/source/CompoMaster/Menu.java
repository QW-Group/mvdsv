// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:09:56
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Menu.java

package CompoMaster;

import Data.Data;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.EventObject;
import javax.swing.*;

// Referenced classes of package CompoMaster:
//            CompoMaster, SignupAdmin, AdminFrame

class Menu extends JMenuBar
{
    class ButtonListener
        implements ActionListener
    {

        public void actionPerformed(ActionEvent e)
        {
            if(e.getSource() == mitNew)
                CompoMaster.newCompo();
            if(e.getSource() == mitRestart)
                CompoMaster.compoRestart();
            if(e.getSource() == mitSave)
                saveDataFile();
            if(e.getSource() == mitSaveAs)
                saveAsDataFile();
            if(e.getSource() == mitLoad)
                CompoMaster.loadCompo();
            if(e.getSource() == mitExportPlayers)
                CompoMaster.exportList(0);
            if(e.getSource() == mitImportPlayers)
            {
                CompoMaster.importList(0);
                if(CompoMaster.frame2 != null)
                    CompoMaster.frame2.dataToGui();
            }
            if(e.getSource() == mitExportMaps)
                CompoMaster.exportList(1);
            if(e.getSource() == mitImportMaps)
                CompoMaster.importList(1);
            if(e.getSource() == mitExit)
                exit();
            if(e.getSource() == mitHtml)
                AdminFrame.showHtml();
        }

        ButtonListener()
        {
        }
    }


    Menu(Data d, boolean isMainAdmin)
    {
        fileMenu = new JMenu();
        trnMenu = new JMenu();
        mitLoad = new JMenuItem();
        mitRestart = new JMenuItem();
        mitNew = new JMenuItem();
        mitSave = new JMenuItem();
        mitSaveAs = new JMenuItem();
        mitImportPlayers = new JMenuItem();
        mitExportPlayers = new JMenuItem();
        mitImportMaps = new JMenuItem();
        mitExportMaps = new JMenuItem();
        mitExit = new JMenuItem();
        mitHtml = new JMenuItem();
        buttonListener = new ButtonListener();
        data = d;
        add(fileMenu);
        fileMenu.add(mitNew);
        if(isMainAdmin)
            fileMenu.add(mitRestart);
        fileMenu.add(mitLoad);
        fileMenu.add(mitSave);
        fileMenu.add(mitSaveAs);
        if(isMainAdmin)
            fileMenu.insertSeparator(5);
        else
            fileMenu.insertSeparator(4);
        fileMenu.add(mitImportPlayers);
        fileMenu.add(mitExportPlayers);
        fileMenu.add(mitImportMaps);
        fileMenu.add(mitExportMaps);
        if(isMainAdmin)
            fileMenu.insertSeparator(10);
        else
            fileMenu.insertSeparator(9);
        fileMenu.add(mitExit);
        mitNew.setText("New competition");
        mitNew.addActionListener(buttonListener);
        mitNew.setMnemonic('N');
        mitRestart.setText("Back to player signup");
        mitRestart.addActionListener(buttonListener);
        mitRestart.setMnemonic('B');
        fileMenu.setMnemonic('F');
        fileMenu.setText("File");
        mitLoad.setText("Load");
        mitLoad.setMnemonic('L');
        mitLoad.addActionListener(buttonListener);
        mitSave.setText("Save");
        mitSave.setMnemonic('S');
        mitSave.addActionListener(buttonListener);
        mitSaveAs.setText("Save as");
        mitSaveAs.setMnemonic('A');
        mitSaveAs.addActionListener(buttonListener);
        mitImportPlayers.setText("Import player list");
        mitImportPlayers.setMnemonic('I');
        mitImportPlayers.addActionListener(buttonListener);
        mitExportPlayers.setText("Export player list");
        mitExportPlayers.setMnemonic('E');
        mitExportPlayers.addActionListener(buttonListener);
        mitImportMaps.setText("Import map list");
        mitImportMaps.setMnemonic('M');
        mitImportMaps.addActionListener(buttonListener);
        mitExportMaps.setText("Export map list");
        mitExportMaps.setMnemonic('O');
        mitExportMaps.addActionListener(buttonListener);
        mitExit.setText("Exit");
        mitExit.setMnemonic('X');
        mitExit.addActionListener(buttonListener);
        if(isMainAdmin)
        {
            add(trnMenu);
            trnMenu.setText("Tournament");
            trnMenu.setMnemonic('T');
            mitHtml.setText("Generate HTML code");
            mitHtml.setMnemonic('H');
            trnMenu.add(mitHtml);
            mitHtml.addActionListener(buttonListener);
        }
        mitImportPlayers.setEnabled(!isMainAdmin);
        mitImportMaps.setEnabled(!isMainAdmin);
    }

    public void saveDataFile()
    {
        CompoMaster.saveCompo();
    }

    public void saveAsDataFile()
    {
        CompoMaster.dataFileName = null;
        CompoMaster.saveCompo();
    }

    public void exit()
    {
        if(CompoMaster.getDataInstance().isChanged)
        {
            int result = JOptionPane.showConfirmDialog(CompoMaster.activeFrame, "Save before quitting?", "Quit program", 1);
            if(result == 0)
                saveDataFile();
            if(result == 2)
                return;
        }
        CompoMaster.exit();
    }

    private JMenu fileMenu;
    private JMenu trnMenu;
    private JMenuItem mitLoad;
    private JMenuItem mitRestart;
    private JMenuItem mitNew;
    private JMenuItem mitSave;
    private JMenuItem mitSaveAs;
    private JMenuItem mitImportPlayers;
    private JMenuItem mitExportPlayers;
    private JMenuItem mitImportMaps;
    private JMenuItem mitExportMaps;
    private JMenuItem mitExit;
    private JMenuItem mitHtml;
    private ButtonListener buttonListener;
    private Data data;











}