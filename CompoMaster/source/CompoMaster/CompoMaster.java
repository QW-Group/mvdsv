/**

$Id: CompoMaster.java,v 1.2 2006/11/27 15:15:45 vvd0 Exp $

**/

package CompoMaster;

import Data.*;
import java.awt.*;
import java.io.*;
import java.net.URL;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

// Referenced classes of package CompoMaster:
//            CmpFilter, SignupAdmin, AdminFrame, ScoreRegDM, 
//            ScoreRegFFA

public class CompoMaster
{

    public CompoMaster()
    {
        packFrame = true;
    }

    public static void main(String args[])
    {
        try
        {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch(Exception exception) { }
        URL url = (CompoMaster.class).getResource("/cm_icon.gif");
        System.out.println(String.valueOf(url));
        icon = Toolkit.getDefaultToolkit().getImage(url);
        cmpFilter = new CmpFilter();
        if(args.length > 0)
        {
            dataFileName = args[0];
            if(!loadCompo(dataFileName))
                compoSetup();
        } else
        {
            compoSetup();
        }
    }

    private static void closeAll()
    {
        if(frame2 != null)
        {
            frame2.dispose();
            frame2 = null;
        }
        if(frame3 != null)
        {
            frame3.dispose();
            frame3 = null;
        }
    }

    public static Image getIcon()
    {
        return icon;
    }

    public static void compoRestart()
    {
        dataFileName = null;
        data.isLaunched = false;
        data.matchList.reset();
        data.removeWalkoverPlayers();
        data.playersToTeams();
        compoSetup();
        frame2.compoLoaded();
    }

    public static void compoSetup()
    {
        closeAll();
        frame2 = new SignupAdmin();
        frame2.pack();
        frame2.show();
        activeFrame = frame2;
    }

    public static void newCompo()
    {
        dataFileName = null;
        data = null;
        compoSetup();
    }

    public static void mainAdmin()
    {
        closeAll();
        frame3 = new AdminFrame("CompoMaster", data);
        data.matchList.setMyContainer(frame3);
        if(data.matchList instanceof CupStructure)
            ((CupStructure)data.matchList).doDetails = false;
        frame3.show();
        activeFrame = frame3;
    }

    public static boolean loadCompo()
    {
        File f = loadBrowser();
        if(f != null)
        {
            String file = f.getAbsolutePath();
            return loadCompo(file);
        } else
        {
            return false;
        }
    }

    public static void saveCompo()
    {
        if(!data.isLaunched)
            frame2.guiToData();
        FileDialog f = new FileDialog(activeFrame);
        if(dataFileName == null)
        {
            f.setDirectory(lastBrowsedDir);
            f.setMode(1);
            f.setFilenameFilter(cmpFilter);
            f.show();
            lastBrowsedDir = f.getDirectory();
            if(f.getFile() == null)
                return;
            dataFileName = lastBrowsedDir + f.getFile();
        }
        boolean result = Data.save(dataFileName, data);
        if(data.getExtData() != null)
            result |= Data.save(lastBrowsedDir + data.getInDataFilename(), data.getExtData());
        if(!result)
            JOptionPane.showMessageDialog(frame2, "Unable to write to file", "I/O Error", 0);
        else
            data.isChanged = false;
    }

    public static boolean loadCompo(String fname)
    {
        boolean res = false;
        Data d = Data.load(fname);
        if(d != null)
            dataFileName = fname;
        else
            JOptionPane.showMessageDialog(frame2, "Unable to read file.", "I/O Error", 0);
        if(d != null)
        {
            res = true;
            data = d;
            File ftemp = new File(fname);
            String fileSep = System.getProperty("file.separator");
            try
            {
                loadedFileDir = ftemp.getCanonicalPath();
                int i = loadedFileDir.lastIndexOf(fileSep);
                if(i > 0)
                    loadedFileDir = loadedFileDir.substring(0, i + 1);
            }
            catch(IOException e)
            {
                loadedFileDir = "";
            }
            String extFileName = data.getInDataFilename();
            if(extFileName != null)
            {
                Data extd = Data.load(loadedFileDir + extFileName);
                if(extd == null)
                    JOptionPane.showMessageDialog(frame2,
						new String((new StringBuffer("Unable to load qualifier tournament '"))
							.append(extFileName)
							.append("'. If you need to see the contestants from this file, please make sure the file resides in the same directory as this file.")),
						"Linked file not found", 0);
                else
                    data.setExtData(extd);
            }
            if(d.isLaunched)
            {
                mainAdmin();
            } else
            {
                compoSetup();
                frame2.compoLoaded();
            }
        }
        return res;
    }

    public static File loadBrowser()
    {
        FileDialog f = new FileDialog(activeFrame);
        f.setFilenameFilter(cmpFilter);
        f.setMode(0);
        f.setDirectory(lastBrowsedDir);
        f.show();
        lastBrowsedDir = f.getDirectory();
        if(f.getFile() == null)
            return null;
        else
            return new File(lastBrowsedDir + f.getFile());
    }

    public static void importList(int whatToLoad)
    {
        FileDialog f = new FileDialog(activeFrame);
        f.setMode(0);
        f.show();
        if(f.getFile() != null)
            try
            {
                BufferedReader inFile = new BufferedReader(new FileReader(f.getDirectory() + f.getFile()));
                if(whatToLoad == 0)
                {
                    RealPlayer p[] = null;
                    data.importPlayerList(inFile);
                } else
                {
                    data.importMapList(inFile);
                    if(activeFrame instanceof SignupAdmin)
                        frame2.dataToGui();
                }
            }
            catch(IOException ioexception) { }
    }

    public static void exportList(int whatToExport)
    {
        FileDialog f = new FileDialog(activeFrame);
        f.setMode(1);
        f.show();
        if(f.getFile() != null)
            try
            {
                FileWriter outFile = new FileWriter(f.getDirectory() + f.getFile());
                if(whatToExport == 0)
                    RealPlayer.exportPlayerList(data, outFile);
                else
                    data.exportMapList(outFile);
            }
            catch(IOException ioexception) { }
    }

    public static void edit(Match match)
    {
        if(match instanceof DeathMatch)
        {
            ScoreRegDM regFrame = new ScoreRegDM((DeathMatch)match, frame3);
            regFrame.pack();
            regFrame.show();
        }
        if(match instanceof FFAGroup)
        {
            ScoreRegFFA regFrame = new ScoreRegFFA((FFAGroup)match, frame3);
            regFrame.pack();
            regFrame.show();
        }
    }

    public static void setDataInstance(Data d)
    {
        data = d;
        Data.setCurrentInstance(d);
    }

    public static Data getDataInstance()
    {
        return data;
    }

    public static void exit()
    {
        System.exit(0);
    }

    public static final String ICON_FILENAME = "/cm_icon.gif";
    public static final int PLAYERLIST = 0;
    public static final int MAPLIST = 1;
    boolean packFrame;
    public static SignupAdmin frame2;
    public static AdminFrame frame3;
    public static Frame activeFrame;
    public static final String defaultFileName = "compo1.cmp";
    public static String dataFileName;
    public static String loadedFileDir = ".\\";
    private static CmpFilter cmpFilter;
    public static String lastBrowsedDir = ".\\";
    private static Image icon;
    private static Data data;

}