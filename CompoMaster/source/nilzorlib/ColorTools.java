// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:18:32
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   ColorTools.java

package nilzorlib.diverse;

import java.awt.Color;

public class ColorTools
{

    public ColorTools()
    {
    }

    public static Color highlightColor(Color c, int weight, int direction, boolean force)
    {
        int r = c.getRed();
        int g = c.getGreen();
        int b = c.getBlue();
        if(!force)
        {
            int trueChange = Math.abs(direction * 256 * 3 - (r + g + b));
            if(trueChange < (weight * 3) / 2)
                return highlightColor(c, weight, 1 - direction, true);
        }
        if(direction == 0)
            weight -= 2 * weight;
        r += weight;
        g += weight;
        b += weight;
        if(r < 0)
            r = 0;
        if(g < 0)
            g = 0;
        if(b < 0)
            b = 0;
        if(r > 255)
            r = 255;
        if(g > 255)
            g = 255;
        if(b > 255)
            b = 255;
        return new Color(r, g, b);
    }

    public static final int BRIGHTER = 1;
    public static final int DARKER = 0;

}