// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:30
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   FDECup.java

package Data;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RectangularShape;
import java.awt.image.BufferedImage;
import nilzorlib.diverse.ColorTools;
import nilzorlib.diverse.ImageTools;

// Referenced classes of package Data:
//            CupStructure, DeathMatch, WinnerOf, MatchPlacingInfo, 
//            LoserOf, RealPlayer, Rankable, MatchList, 
//            Data, Match

public class FDECup extends CupStructure
    implements Rankable
{
    class DrawMeasures
    {

        private int getFirstLbLength()
        {
            int firstLbLength = frLength / 2;
            for(int i = numSERounds; i > 0; i--)
                firstLbLength /= 2;

            return firstLbLength;
        }

        Point wbPos;
        Point lbPos;
        Point sePos;
        Dimension wbDim;
        Dimension lbDim;
        Dimension seDim;
        int bracketSeparator;
        Dimension totalSize;

        DrawMeasures()
        {
            int numRounds = ((Match) (match[ptrFin])).placingInfo.roundNr + 1;
            int numBracketHeaders = 1;
            if(numSERounds > 0)
            {
                seDim = new Dimension(numSERounds * 158, 0);
                sePos = new Point(25, 0);
                numBracketHeaders = 2;
            } else
            {
                seDim = new Dimension(0, 0);
                sePos = new Point(0, 0);
            }
            totalSize = new Dimension();
            int w = (numRounds - numSERounds) * 316 - 158;
            int frHeight = frLength * 60 + 20 + 10;
            if(allSingle)
            {
                totalSize.width = sePos.x + seDim.width;
                totalSize.height = frHeight;
                wbDim = new Dimension(0, frHeight);
                wbPos = new Point(0, 0);
                lbDim = new Dimension(0, 0);
                lbPos = new Point(0, 0);
            } else
            {
                wbPos = new Point(sePos.x + seDim.width + 25, 0);
                wbDim = new Dimension(w, frHeight);
                if(numSERounds > 0)
                    wbPos.x += 22;
                lbPos = new Point(wbPos.x, wbDim.height);
                lbDim = new Dimension(w, getFirstLbLength() * 60 + 20 + 10);
                if(lbDim.height < 120)
                    lbDim.height = 120;
                bracketSeparator = wbDim.height;
                totalSize.width = wbDim.width + wbPos.x;
                totalSize.height = wbDim.height + lbDim.height;
            }
            seDim.height = totalSize.height;
        }
    }


    public FDECup(Data data, int dmWinMode, int numExtraFinales, int numSERounds)
    {
        allSingle = false;
        super.dmWinMode = dmWinMode;
        super.numFinales = numExtraFinales + 1;
        this.numSERounds = numSERounds;
        super.data = data;
    }

    public Match getMatch(int nr)
    {
        if(nr < 0 || nr >= match.length)
            return null;
        else
            return match[nr];
    }

    public void initCompo()
    {
        super.numWalkovers = getNumWalkovers();
        frLength = (super.numWalkovers + super.data.getNumPlayers()) / 2;
        wbLength = frLength - 1;
        lbLength = wbLength * 2 + 2;
        int i;
        for(i = numSERounds; i > 0; i--)
            lbLength /= 2;

        lbLength -= 2;
        ptrR2 = frLength;
        int j;
        if(lbLength <= 0)
        {
            allSingle = true;
            finLength = super.numFinales;
            ptrFin = (wbLength + frLength) - 1;
            ptrLB = ptrFin + 1;
        } else
        {
            finLength = super.numFinales + 1;
            ptrLB = ptrR2 + wbLength;
            ptrFin = ptrLB + lbLength;
            i = numSERounds;
            for(j = frLength; i > 0; j /= 2)
            {
                ptrWB += j;
                i--;
            }

        }
        super.numMatches = ptrFin + super.numFinales;
        match = new DeathMatch[super.numMatches + 1];
        initFirstRound(match, frLength);
        if(!allSingle)
            if(numSERounds > 0)
                for(i = 0; i < frLength; i++)
                    match[i].name = "SE-".concat(String.valueOf(String.valueOf(((Match) (match[i])).name)));

            else
                for(i = 0; i < frLength; i++)
                    match[i].name = "WB-".concat(String.valueOf(String.valueOf(((Match) (match[i])).name)));

        int round = 1;
        int roundSize = frLength / 2;
        bracketPtr = 0;
        int absPtr = ptrR2;
        j = 0;
        while(absPtr < ptrLB) 
        {
            for(i = 0; i < roundSize;)
            {
                if(absPtr < ptrWB)
                    match[absPtr] = new DeathMatch("SE-".concat(String.valueOf(String.valueOf(String.valueOf(absPtr + 1)))));
                else
                    match[absPtr] = new DeathMatch("WB-".concat(String.valueOf(String.valueOf(String.valueOf((absPtr - ptrWB) + 1)))));
                match[absPtr].setPlayer(0, new WinnerOf(super.data.matchList.getMatch(j)));
                match[absPtr].setPlayer(1, new WinnerOf(super.data.matchList.getMatch(j + 1)));
                match[absPtr].placingInfo = new MatchPlacingInfo(round, roundSize, i, bracketPtr, 1);
                i++;
                j += 2;
                absPtr++;
                bracketPtr++;
            }

            round++;
            roundSize /= 2;
        }
        roundSize = frLength / (int)Math.pow(2D, (double)numSERounds + (double)1);
        bracketPtr = 0;
        dropperPtr = 0;
        i = numSERounds;
        for(j = frLength; i > 0; j /= 2)
        {
            dropperPtr += j;
            i--;
        }

        for(round = numSERounds * 2; absPtr < ptrFin; round++)
        {
            for(j = 0; j < roundSize;)
            {
                match[absPtr] = new DeathMatch("LB-".concat(String.valueOf(String.valueOf(String.valueOf(bracketPtr + 1)))));
                if(round == numSERounds * 2)
                {
                    match[absPtr].setPlayer(0, new LoserOf(super.data.matchList.getMatch(dropperPtr)));
                    match[absPtr].setPlayer(1, new LoserOf(super.data.matchList.getMatch(dropperPtr + 1)));
                    dropperPtr += 2;
                    match[absPtr].placingInfo = new MatchPlacingInfo(numSERounds + 1, roundSize, j, bracketPtr, 2);
                } else
                {
                    if(round % 2 == 0)
                    {
                        match[absPtr].placingInfo = new MatchPlacingInfo(round / 2 + 1, roundSize, j, bracketPtr, 2);
                        match[absPtr].setPlayer(0, new WinnerOf(super.data.matchList.getMatch((absPtr - roundSize * 2) + j)));
                        match[absPtr].setPlayer(1, new WinnerOf(super.data.matchList.getMatch((absPtr - roundSize * 2) + 1 + j)));
                    }
                    if(round % 2 == 1)
                    {
                        match[absPtr].placingInfo = new MatchPlacingInfo(round / 2 + 1, roundSize, j, bracketPtr, 3);
                        match[absPtr].setPlayer(0, new LoserOf(super.data.matchList.getMatch(getDropper(((Match) (match[absPtr])).placingInfo))));
                        match[absPtr].setPlayer(1, new WinnerOf(super.data.matchList.getMatch(absPtr - roundSize)));
                    }
                }
                j++;
                absPtr++;
                bracketPtr++;
            }

            if(round % 2 == 1)
                roundSize /= 2;
        }

        if(allSingle)
        {
            match[ptrFin].setName("Final");
            match[ptrFin].placingInfo = new MatchPlacingInfo(((Match) (match[ptrFin])).placingInfo.roundNr, super.numFinales, 0, ptrFin, 5);
        } else
        {
            match[ptrFin] = new DeathMatch("Final");
            match[ptrFin].setPlayer(0, new WinnerOf(super.data.matchList.getMatch(ptrLB - 1)));
            match[ptrFin].setPlayer(1, new WinnerOf(super.data.matchList.getMatch(ptrFin - 1)));
            match[ptrFin].placingInfo = new MatchPlacingInfo(round / 2 + 1, super.numFinales, 0, 0, 4);
        }
        if(super.numFinales > 1)
            if(allSingle)
            {
                match[ptrFin + 1] = new DeathMatch("Third place");
                match[ptrFin + 1].setPlayer(0, new LoserOf(super.data.matchList.getMatch(ptrFin - 2)));
                match[ptrFin + 1].setPlayer(1, new LoserOf(super.data.matchList.getMatch(ptrFin - 1)));
                match[ptrFin + 1].placingInfo = new MatchPlacingInfo(((Match) (match[ptrFin])).placingInfo.roundNr, super.numFinales, 1, ptrFin + 1, 5);
            } else
            {
                match[ptrFin + 1] = new DeathMatch("5th place");
                match[ptrFin + 1].setPlayer(0, new LoserOf(super.data.matchList.getMatch(ptrFin - 4)));
                match[ptrFin + 1].setPlayer(1, new LoserOf(super.data.matchList.getMatch(ptrFin - 3)));
                match[ptrFin + 1].placingInfo = new MatchPlacingInfo(round / 2 + 1, super.numFinales, 1, 0, 4);
            }
        if(super.numFinales > 2)
        {
            match[ptrFin + 2] = new DeathMatch("7th place");
            match[ptrFin + 2].setPlayer(0, new LoserOf(super.data.matchList.getMatch(ptrFin - 6)));
            match[ptrFin + 2].setPlayer(1, new LoserOf(super.data.matchList.getMatch(ptrFin - 5)));
            match[ptrFin + 2].placingInfo = new MatchPlacingInfo(round / 2 + 1, super.numFinales, 2, 0, 4);
        }
    }

    public int getNumSERounds()
    {
        return numSERounds;
    }

    private int getDropper(MatchPlacingInfo mp)
    {
        int key = ((Match) (match[0])).placingInfo.roundSize / 2;
        int nr = mp.roundIndex * (1 << mp.roundNr);
        int i;
        for(i = 1; i <= mp.roundNr; i++)
        {
            int a = key;
            for(int b = 1; a > 0; b <<= 1)
            {
                if(i % b == 0)
                    if((nr & a) > 0)
                        nr -= a;
                    else
                        nr += a;
                a >>= 1;
            }

        }

        nr >>= mp.roundNr;
        for(i = ptrR2; getMatch(i).placingInfo.roundNr != mp.roundNr; i++);
        return nr + i;
    }

    protected Point getMatchPosition(Match m)
    {
        Point pos = new Point(0, 0);
        MatchPlacingInfo pi = m.placingInfo;
        DrawMeasures meas = getDrawMeasures();
        if(pi.roundNr < numSERounds)
        {
            pos.x = 25 + 158 * pi.roundNr;
        } else
        {
            pos.x = meas.wbPos.x;
            if(pi.roundNr <= numSERounds + 1)
                pos.x += (pi.roundNr - numSERounds) * 158;
            else
                pos.x += (pi.roundNr - numSERounds) * 158 * 2 - 158;
        }
        int ySpace = (int)(Math.pow(2D, m.placingInfo.roundNr) * 60D);
        pos.y = (ySpace / 2 + ySpace * m.placingInfo.roundIndex) - 30;
        if(pi.bracketCode == 3)
            pos.x += 158;
        if(pi.bracketCode == 2 || pi.bracketCode == 3)
        {
            ySpace = (int)(Math.pow(2D, m.placingInfo.roundNr - numSERounds - 1) * 60D);
            pos.y = (ySpace / 2 + ySpace * m.placingInfo.roundIndex) - 30;
            pos.y += meas.bracketSeparator;
        }
        if(pi.bracketCode == 4)
        {
            pos.y = meas.bracketSeparator - 60;
            if(pi.roundIndex == super.numFinales)
                pos.x += 158;
            else
            if(pi.roundIndex > 0)
                pos.y += 60 * pi.roundIndex;
        }
        if(pi.bracketCode == 5)
            pos.y = (ySpace / 2 + pi.roundIndex * 60) - 30;
        pos.x += 5;
        pos.y += 40;
        return pos;
    }

    public Dimension getDrawingSize()
    {
        return getDrawMeasures().totalSize;
    }

    public void drawCompo(Point offset)
    {
        Graphics g = super.compoGraphics;
        if(imgBackground == null)
            generateBackground();
        ((Graphics2D)g).drawImage(imgBackground, 0, 0, null);
        if(match[ptrFin].isPlayed())
            if(match[ptrFin].getScore(0) < match[ptrFin].getScore(1))
            {
                if(match[match.length - 1] == null)
                {
                    match[match.length - 1] = new DeathMatch(new WinnerOf(super.data.matchList.getMatch(ptrFin)), new LoserOf(super.data.matchList.getMatch(ptrFin)), "Final rematch");
                    match[match.length - 1].placingInfo = new MatchPlacingInfo(((Match) (match[ptrFin])).placingInfo.roundNr, ((Match) (match[ptrFin])).placingInfo.roundSize, super.numFinales, ((Match) (match[ptrFin])).placingInfo.bracketIndex, 4);
                    super.numMatches++;
                }
            } else
            if(match[match.length - 1] != null)
            {
                match[match.length - 1] = null;
                super.numMatches--;
            }
        for(int i = 0; i < super.numMatches; i++)
            if(match[i] != null)
            {
                Point pos = getMatchPosition(match[i]);
                drawMatch(g, offset, pos, match[i]);
            }

    }

    private void generateBackground()
    {
        int colDiff = 10;
        DrawMeasures meas = getDrawMeasures();
        int hWb = meas.wbDim.height;
        int hLb = meas.lbDim.height;
        imgBackground = new BufferedImage(meas.totalSize.width, meas.totalSize.height, 1);
        Graphics2D g = imgBackground.createGraphics();
        String cname = super.data.getCompoName();
        if(cname == null);
        int x = 25;
        int numRounds = getNumRounds();
        Color col = CupStructure.SECOL;
        for(int i = 0; i < numSERounds; i++)
        {
            col = CupStructure.SECOL;
            if(i % 2 == 0)
                col = ColorTools.highlightColor(col, colDiff, 0, false);
            g.setColor(col);
            g.fillRect(x, 0, 158, hWb + hLb);
            g.setColor(Color.black);
            g.drawRect(x, 0, 158, hWb + hLb);
            drawTextBox(g, CupStructure.HEADERBGCOL, String.valueOf(String.valueOf((new StringBuffer("Round ")).append(i + 1))), x, 0, 0);
            x += 158;
        }

        int rest = meas.wbPos.x - x;
        if(rest > 0)
        {
            g.setColor(col);
            g.fillRect(x, 0, rest, hWb + hLb);
        }
        x = meas.wbPos.x;
        for(int i = numSERounds; i < numRounds; i++)
        {
            Color col1 = CupStructure.WBCOL;
            Color col2 = CupStructure.LBCOL;
            if(i % 2 == 0)
            {
                col1 = ColorTools.highlightColor(col1, colDiff, 0, false);
                col2 = ColorTools.highlightColor(col2, colDiff, 0, false);
            }
            g.setColor(col1);
            g.fillRect(x, 0, 316, hWb);
            g.setColor(col2);
            g.fillRect(x, hWb, 316, hLb);
            g.setColor(Color.black);
            g.drawRect(x, 0, 316, hWb);
            g.drawRect(x, hWb, 316, hLb);
            drawTextBox(g, CupStructure.HEADERBGCOL, String.valueOf(String.valueOf((new StringBuffer("Round ")).append(i + 1))), x, 0, 0);
            if(i > numSERounds)
                x += 316;
            else
                x += 158;
        }

        if(numSERounds > 0)
        {
            BufferedImage imgTemp = createBracketHeader("Single elimination", hWb + hLb, ColorTools.highlightColor(CupStructure.SECOL, 20, 1, true));
            g.drawImage(imgTemp, null, 0, 0);
        }
        if(numSERounds < numRounds)
        {
            BufferedImage imgTemp = createBracketHeader("Winners bracket", hWb + 1, ColorTools.highlightColor(CupStructure.WBCOL, 20, 1, true));
            g.drawImage(imgTemp, null, meas.wbPos.x - 25, meas.wbPos.y);
            imgTemp = createBracketHeader("Losers bracket", hLb, ColorTools.highlightColor(CupStructure.LBCOL, 20, 1, true));
            g.drawImage(imgTemp, null, meas.lbPos.x - 25, meas.lbPos.y);
        }
        drawVersionInfo(g);
    }

    public int getNumRounds()
    {
        return ((Match) (match[ptrFin])).placingInfo.roundNr + 1;
    }

    public String getRankText(int rank)
    {
        return "(from cup)";
    }

    public Object getPlayerAt(int rank)
    {
        if(allSingle)
        {
            int mPtr = (ptrFin - super.data.outRankLow * 2) + rank + 1;
            if(match[mPtr].isPlayed())
                return (RealPlayer)match[mPtr].getWinner();
        }
        return null;
    }

    private BufferedImage createBracketHeader(String text, int height, Color col)
    {
        int w = height;
        int h = 25;
        BufferedImage temp = new BufferedImage(w, h, 6);
        Graphics g_temp = temp.getGraphics();
        int fontSize = 16;
        int sw;
        int sh;
        Font font;
        do
        {
            font = new Font(MatchList.getMainFont().getName(), 1, fontSize);
            FontMetrics fm = g_temp.getFontMetrics(font);
            Rectangle2D r = fm.getStringBounds(text, g_temp);
            sw = (int)r.getWidth();
            sh = (int)r.getHeight();
            fontSize--;
        } while(sw > w);
        g_temp.setFont(font);
        g_temp.setColor(col);
        g_temp.fillRect(0, 0, w, h);
        g_temp.setColor(Color.white);
        g_temp.drawString(text, (w - sw) / 2, h - (h - sh) / 2 - 5);
        g_temp.setColor(Color.black);
        g_temp.drawRect(0, 0, w - 1, h);
        return ImageTools.rotate90(temp, 0);
    }

    private DrawMeasures getDrawMeasures()
    {
        if(drawMeasures == null)
            drawMeasures = new DrawMeasures();
        return drawMeasures;
    }

    private static final long serialVersionUID = 0x1b8aaa0475b2b081L;
    public DeathMatch match[];
    private boolean allSingle;
    private int ptrR2;
    private int ptrLB;
    private int ptrFin;
    private int bracketPtr;
    private int frLength;
    private int wbLength;
    private int lbLength;
    private int finLength;
    private int dropperPtr;
    private int numSERounds;
    private int ptrWB;
    private transient DrawMeasures drawMeasures;
    private transient BufferedImage imgBackground;





}