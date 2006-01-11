// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:13:11
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   CompoViewer.java

package CompoViewer;

import Data.*;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.EventObject;
import nilzorlib.diverse.FTimer;

// Referenced classes of package CompoViewer:
//            CompoDrawing, CompoSelecter, EditInvoker

public class CompoViewer extends Applet
    implements ActionListener, KeyListener
{

    public CompoViewer()
    {
        NAGONTIME = 3000;
        NAGOFFTIME = 15000;
        showVersionInfo = false;
        teller = 0;
        selected = 0;
        errors = "";
        errorsDrawn = false;
        firstTime = true;
        loadFinished = false;
        numCompos = 0;
        data = new Data[10];
        filename = new String[10];
    }

    public Data getData()
    {
        return data[0];
    }

    public void init()
    {
        setLayout(new FlowLayout(0, 0, 0));
        parseParameters();
        setBackground(bgColor);
    }

    private Color getColorParameter(String varName)
    {
        String s = getParameter(varName);
        if(s != null)
        {
            if(s.charAt(0) != '#' || s.length() < 7)
                return null;
            try
            {
                int r = Integer.parseInt(s.substring(1, 3), 16);
                int g = Integer.parseInt(s.substring(3, 5), 16);
                int b = Integer.parseInt(s.substring(5), 16);
                Color color = new Color(r, g, b);
                return color;
            }
            catch(NumberFormatException e)
            {
                Color color1 = null;
                return color1;
            }
        } else
        {
            return null;
        }
    }

    private void parseParameters()
    {
        Color col = getColorParameter("MatchColor");
        if(col != null)
        {
            DeathMatch.M_BGCOL1 = col;
            Group.BG1 = col;
        }
        col = getColorParameter("WalkoverColor");
        if(col != null)
        {
            DeathMatch.M_WO_COL = col;
            Group.disabledCol = col;
        }
        col = getColorParameter("CupMatchBorderColor");
        if(col != null)
            DeathMatch.M_FRAMECOL = col;
        col = getColorParameter("CupWBColor");
        if(col != null)
            CupStructure.WBCOL = col;
        col = getColorParameter("CupSEColor");
        if(col != null)
            CupStructure.SECOL = col;
        col = getColorParameter("CupLBColor");
        if(col != null)
            CupStructure.LBCOL = col;
        col = getColorParameter("CupTextBGColor");
        if(col != null)
            CupStructure.HEADERBGCOL = col;
        col = getColorParameter("CupEmptyMatchColor");
        if(col != null)
            DeathMatch.ABSTRACT_COL = col;
        col = getColorParameter("TextColor1");
        if(col != null)
            CupStructure.TEXTCOL = col;
        col = getColorParameter("TextColor2");
        if(col != null)
        {
            DeathMatch.M_TEXTCOL = col;
            DeathMatch.IDCOLOR = col;
        }
        if(getParameter("Debug") != null)
            Data.debug = true;
        matches = getParameter("MatchesFile");
        teams = getParameter("TeamsFile");
        editUrl = getParameter("EditURL");
        if(editUrl == null)
        {
            editUrl = getParameter("DetailsURL");
            if(editUrl == null)
                Data.doDetailsButton = false;
        }
        bgColor = DEFAULT_BGCOL;
        col = getColorParameter("BgColor");
        if(col != null)
            bgColor = col;
        if(bgColor.getRed() + bgColor.getBlue() + bgColor.getGreen() > 384)
            textColor = Color.black;
        else
            textColor = Color.white;
    }

    private boolean loadData()
    {
        compo = new CompoDrawing[10];
        pi = new ProgressIndicator[10];
        int compoLink[] = new int[10];
        for(int i = 0; i < 10; i++)
        {
            String _dataFile = getParameter("DataFile".concat(String.valueOf(String.valueOf(i))));
            if(_dataFile != null)
            {
                filename[numCompos] = _dataFile;
                compoLink[numCompos] = -1;
                pi[numCompos] = new ProgressIndicator();
                pi[numCompos].setEventHandler(this);
                pi[numCompos].setGoal(1);
                pi[numCompos].setProgress(0);
                numCompos++;
            }
        }

        for(int i = 0; i < numCompos;)
            try
            {
                url = new URL(getDocumentBase(), filename[i]);
                data[i] = Data.load(url, pi[i]);
                if(data[i].getInDataFilename() != null)
                {
                    filename[numCompos] = data[i].getInDataFilename();
                    pi[numCompos] = new ProgressIndicator();
                    pi[numCompos].setEventHandler(this);
                    pi[numCompos].setGoal(1);
                    pi[numCompos].setProgress(0);
                    compoLink[i] = numCompos;
                    numCompos++;
                }
                continue;
            }
            catch(MalformedURLException e)
            {
                errors = String.valueOf(errors) + String.valueOf(String.valueOf(String.valueOf((new StringBuffer("Error: Malformed URL exception (")).append(url).append(") "))));
                errors = String.valueOf(errors) + String.valueOf(e.getMessage());
                continue;
            }
            catch(Exception e)
            {
                if(Data.errors.endsWith("Error=1"))
                    errors = String.valueOf(errors) + String.valueOf(String.valueOf(String.valueOf((new StringBuffer("File not found (")).append(url).append(")."))));
                else
                    errors = String.valueOf(errors) + String.valueOf(String.valueOf(String.valueOf((new StringBuffer("Error while loading file. (")).append(url).append(") - ").append(Data.errors))));
                if(Data.debug)
                {
                    errors = String.valueOf(errors) + String.valueOf(e.toString());
                    StringWriter temp = new StringWriter();
                    PrintWriter p_temp = new PrintWriter(temp);
                    e.printStackTrace(p_temp);
                    errors = String.valueOf(errors) + String.valueOf(String.valueOf(temp.toString()) + String.valueOf(e.toString()));
                }
                i++;
            }

        if(!errors.equals(""))
            return false;
        for(int i = 0; i < numCompos; i++)
            if(compoLink[i] != -1)
                data[i].setExtData(data[compoLink[i]]);

        return true;
    }

    private void addComponents()
    {
        try
        {
            if(numCompos > 0)
            {
                if(data[0].matchList.getMatch(0) == null)
                {
                    errors = String.valueOf(String.valueOf(errors)).concat("Error: No groups defined");
                    return;
                }
                Dimension winSize = getSize();
                setLayout(new BorderLayout(0, 0));
                if(numCompos > 1)
                {
                    CompoSelecter cs = new CompoSelecter(this);
                    add(cs, "North");
                    cs.doLayout();
                    winSize.height -= 30;
                }
                int maxX = 0;
                int maxY = 0;
                for(int i = 0; i < numCompos; i++)
                {
                    compo[i] = new CompoDrawing(data[i], winSize);
                    compo[i].addKeyListener(this);
                    data[i].matchList.setEditListener(new EditInvoker(editUrl, "".concat(String.valueOf(String.valueOf(i))), data[i], this));
                    Dimension thisDim = data[i].matchList.getSize();
                    if(thisDim.width > maxX)
                        maxX = thisDim.width;
                    if(thisDim.height > maxY)
                        maxY = thisDim.height;
                }

                Data.setCurrentInstance(compo[selected].data);
                compo[selected].repaint();
                if(maxX > winSize.width || maxY > winSize.height)
                {
                    sp = new ScrollPane(0);
                    sp.add(compo[selected]);
                    add(sp, "Center");
                    doLayout();
                    sp.doLayout();
                } else
                {
                    add(compo[selected]);
                    doLayout();
                }
            }
            addKeyListener(this);
        }
        catch(Exception e)
        {
            errors = String.valueOf(errors) + String.valueOf(e.toString());
        }
    }

    public Data getData(int i)
    {
        if(i < data.length)
            return data[i];
        else
            return null;
    }

    public void debugPrint(String s)
    {
        applet = this;
    }

    public boolean checkVersion()
    {
        String ver = System.getProperty("java.vm.version");
        System.out.println(ver);
        double d_ver;
        if(ver.length() > 2)
            d_ver = Double.parseDouble(ver.substring(0, 3));
        else
            d_ver = Double.parseDouble(ver);
        if(d_ver >= 1.3D)
        {
            return true;
        } else
        {
            errors = String.valueOf(String.valueOf((new StringBuffer("Fatal: Java VM too old (")).append(ver).append("). ").append("You need at least Java SE v1.3 to view this applet.   ").append("Please download the latest version from http://java.sun.com")));
            return false;
        }
    }

    public void paint(Graphics g)
    {
        super.paint(g);
        if(!loadFinished)
        {
            int textoffset = 20;
            g.setColor(textColor);
            g.drawString(Data.VERSION, 20, textoffset);
            g.drawString(Data.VERSION_2, 20, textoffset + 20);
            for(int i = 0; i < 10; i++)
                if(pi != null && pi[i] != null)
                    drawProgress(g, i);

        }
        if(firstTime)
        {
            firstTime = false;
            if(checkVersion())
            {
                Thread loadThread = new Thread() {

                    public void run()
                    {
                        boolean success = loadData();
                        if(success)
                            addComponents();
                        loadFinished = true;
                        repaint();
                    }

                };
                loadThread.start();
            } else
            {
                loadFinished = true;
                repaint();
            }
        }
        if(loadFinished && !errors.equals(""))
        {
            if(!errorsDrawn && Data.debug)
                errors = String.valueOf(errors) + String.valueOf(" - ".concat(String.valueOf(String.valueOf(Data.errors))));
            errorsDrawn = true;
            if(compo != null && compo[0] != null)
                remove(compo[0]);
            if(bgColor.getGreen() + bgColor.getRed() + bgColor.getBlue() > 512)
                g.setColor(Color.black);
            else
                g.setColor(Color.white);
            g.drawString(Data.VERSION, 5, 20);
            g.drawString(Data.VERSION_2, 5, 40);
            for(int a = 0; a < errors.length(); a += 90)
            {
                int b = a + 90;
                if(b > errors.length())
                    b = errors.length();
                g.drawString(errors.substring(a, b), 5, 60 + (a / 90) * 20);
            }

        }
        if(showVersionInfo)
            drawVersionInfo(g);
    }

    public void setSelected(int i)
    {
        Container cnt;
        if(sp != null)
            cnt = sp;
        else
            cnt = this;
        cnt.remove(compo[selected]);
        selected = i;
        cnt.add(compo[selected]);
        cnt.doLayout();
        Data.setCurrentInstance(compo[selected].data);
        compo[selected].repaint();
    }

    public void actionPerformed(ActionEvent e)
    {
        if(e.getSource() == nagTimer)
        {
            if(nagOn)
            {
                nagTimer = new FTimer(NAGOFFTIME, this);
                nagTimer.setRepeats(false);
                nagTimer.start();
            } else
            {
                nagTimer = new FTimer(NAGONTIME, this);
                nagTimer.setRepeats(false);
                nagTimer.start();
            }
            nagOn = !nagOn;
            compo[selected].repaint();
        } else
        {
            repaint();
        }
    }

    private void drawProgress(Graphics piGraf, int i)
    {
        int ROUNDNESS = 8;
        Dimension size = getSize();
        int y = 100 + i * 45;
        int x = 20;
        Color left = new Color(70, 70, 70);
        Color done = new Color(100, 100, 255);
        double fact = (double)pi[i].getProgress() / (double)pi[i].getGoal();
        piGraf.setColor(textColor);
        piGraf.drawString(String.valueOf(String.valueOf((new StringBuffer("Loading tournament ")).append(i + 1))), x, y - 5);
        piGraf.setColor(Color.black);
        piGraf.fillRoundRect(x - 1, y - 1, 402, 27, ROUNDNESS, ROUNDNESS);
        piGraf.setColor(left);
        piGraf.fillRoundRect(x, y, 400, 25, ROUNDNESS, ROUNDNESS);
        piGraf.setColor(done);
        piGraf.fillRoundRect(x, y, (int)((double)400 * fact), 25, ROUNDNESS, ROUNDNESS);
    }

    public void keyReleased(KeyEvent e)
    {
        if(e.getKeyChar() == 'I' || e.getKeyChar() == 'i' && showVersionInfo)
        {
            showVersionInfo = false;
            compo[selected].repaint();
            repaint();
        }
    }

    public void keyTyped(KeyEvent keyevent)
    {
    }

    public void keyPressed(KeyEvent e)
    {
        if(e.getKeyChar() == 'I' && !showVersionInfo)
        {
            showVersionInfo = true;
            compo[selected].repaint();
            repaint();
        }
    }

    private void drawVersionInfo(Graphics gasdf)
    {
        int w = 280;
        int h = 50;
        int x = (getSize().width - w) / 2;
        int y = (getSize().height - h) / 2;
        if(y > 100)
            y = 100;
        if(x > 100)
            x = 100;
        Graphics g = compo[selected].getGraphics();
        g.setColor(new Color(80, 80, 80));
        g.fillRect(x, y, w, h);
        g.setColor(new Color(255, 255, 255));
        g.drawRect(x - 1, y - 1, w + 2, h + 2);
        g.setColor(Color.white);
        g.drawString(Data.VERSION, x + 5, y + 20);
        g.drawString(Data.VERSION_2, x + 5, y + 40);
    }

    public static final int MAXCOMPOS = 10;
    public static final Color DEFAULT_BGCOL;
    private int NAGONTIME;
    private int NAGOFFTIME;
    public static final int PI_YOFFSET = 100;
    public static final int PI_HEIGHT = 25;
    public static final int PI_WIDTH = 400;
    public static final int PI_YSPACING = 20;
    public static final boolean nonPublic = false;
    public static CompoViewer applet;
    private ScrollPane sp;
    private boolean showVersionInfo;
    private int teller;
    CompoDrawing compo[];
    private int selected;
    private String errors;
    private boolean errorsDrawn;
    private boolean firstTime;
    private boolean loadFinished;
    private int numCompos;
    public static Color bgColor;
    private static Color textColor;
    FTimer nagTimer;
    public static boolean nagOn;
    Match selectedMatch;
    Data data[];
    String matches;
    String teams;
    String filename[];
    String editUrl;
    URL url;
    Graphics piGraf;
    ProgressIndicator pi[];

    static 
    {
        DEFAULT_BGCOL = Color.black;
    }



}