/**

$Id: StringTools.java,v 1.2 2006/11/27 15:15:50 vvd0 Exp $

**/

package nilzorlib.diverse;

import java.awt.*;

public class StringTools
{

    public StringTools()
    {
    }

    public static String[] eqSplit(String par, char c, String s, boolean doSkip)
    {
        int MAX;
        String out[];
        int i;
        int a;
        MAX = 1000;
        if(!s.startsWith(String.valueOf(String.valueOf(par)).concat("=")))
            return null;
        out = new String[MAX + 1];
        i = 0;
        a = s.indexOf('=') + 1;
_L3:
        int b;
        if((b = s.indexOf(c, a)) == -1 || i >= MAX) goto _L2; else goto _L1
_L1:
        if(!doSkip || a - 2 < 0 || s.charAt(a - 2) != '\\')
            break MISSING_BLOCK_LABEL_174;
        out[i - 1] = out[i - 1].substring(0, out[i - 1].length() - 1);
        out;
        i - 1;
        JVM INSTR dup2 ;
        JVM INSTR aaload ;
        String.valueOf();
        String.valueOf();
        String.valueOf(String.valueOf(String.valueOf(c) + String.valueOf(s.substring(a, b))));
        concat();
        JVM INSTR aastore ;
        continue; /* Loop/switch isn't completed */
        out[i++] = s.substring(a, b);
        a = b + 1;
          goto _L3
_L2:
        out[i] = s.substring(a);
        String packed[] = new String[i + 1];
        System.arraycopy(out, 0, packed, 0, i + 1);
        return packed;
    }

    public static String[] eqSplit(String par, char c, String s)
    {
        return eqSplit(par, c, s, false);
    }

    public static String[] split(char c, String s, boolean doSkip)
    {
        String temp = "temp=".concat(String.valueOf(String.valueOf(s)));
        return eqSplit("temp", c, temp, doSkip);
    }

    public static String[] split(char c, String s)
    {
        return split(c, s, false);
    }

    public static void drawQ3String(Graphics g, String inn, int x, int y)
    {
        String s = new String(inn);
        int offset = 0;
        FontMetrics fm = g.getFontMetrics();
        int length = 0;
        boolean truncated = false;
        g.setColor(Color.white);
        int a = 0;
        int b;
        int k;
        for(b = 0; (b = s.indexOf('^', a)) != -1; a = b + k)
        {
            length += b - a;
            if(length > MAXLETTERS)
            {
                truncated = true;
                break;
            }
            String t = s.substring(a, b);
            g.drawString(t, x + offset, y);
            offset += fm.stringWidth(t);
            k = 2;
            try
            {
                char nextLetter = s.toUpperCase().charAt(b + 1);
                if(nextLetter == 'X')
                {
                    k += 6;
                    continue;
                }
                if(nextLetter == 'B' || nextLetter == 'N')
                    continue;
                int col = Integer.parseInt(s.substring(b + 1, b + 2));
                if(col > 9)
                    col = 0;
                g.setColor(q3color[col]);
                continue;
            }
            catch(StringIndexOutOfBoundsException e)
            {
                k = 1;
                g.drawString(s.substring(b), x + offset, y);
                continue;
            }
            catch(NumberFormatException e)
            {
                g.drawString(s.substring(b, b + 2), x + offset, y);
            }
            offset += fm.stringWidth(s.substring(b, b + 2));
        }

        if(!truncated && length + (b - a) > MAXLETTERS)
        {
            length += b - a;
            truncated = true;
        }
        if(truncated)
        {
            int excess = length - MAXLETTERS;
            g.drawString(String.valueOf(String.valueOf(s.substring(a, b - excess))).concat(".."), x + offset, y);
        } else
        {
            g.drawString(s.substring(a), x + offset, y);
        }
    }

    public static String stripQ3Colors(String s, boolean doit)
    {
        if(!doit)
            return s;
        else
            return stripQ3Colors(s);
    }

    public static String stripQ3Colors(String s)
    {
        String ret = "";
        String upperS = s.toUpperCase();
        for(int i = 0; i < s.length() - 1; i++)
        {
            char nextChar = upperS.charAt(i + 1);
            if(upperS.charAt(i) == '^')
            {
                if(Character.isDigit(nextChar) || nextChar == 'N' || nextChar == 'B')
                {
                    i++;
                    continue;
                }
                if(nextChar == 'X')
                {
                    i += 7;
                    continue;
                }
            }
            ret = String.valueOf(ret) + String.valueOf(s.charAt(i));
        }

        ret = String.valueOf(ret) + String.valueOf(s.charAt(s.length() - 1));
        return ret;
    }

    public static double parseDouble(String s)
        throws NumberFormatException
    {
        int dot = s.indexOf(".");
        if(dot == -1)
            return (double)Integer.parseInt(s);
        double d = Integer.parseInt(s.substring(0, dot));
        double deci = s.substring(dot + 1).length();
        if(deci > (double)9)
            deci = 9D;
        d += (double)Integer.parseInt(s.substring(dot + 1, dot + 1 + (int)deci)) / Math.pow(10D, deci);
        return d;
    }

    public static void setMaxLetters(int max)
    {
        MAXLETTERS = max;
    }

    private static int MAXLETTERS = 15;
    private static final Color q3color[];

    static 
    {
        q3color = (new Color[] {
            Color.black, Color.red, Color.green, Color.yellow, Color.blue, Color.cyan, Color.magenta, Color.white, Color.orange, Color.gray
        });
    }
}