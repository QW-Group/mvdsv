// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:18:40
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   ImageTools.java

package nilzorlib.diverse;

import java.awt.image.BufferedImage;

public class ImageTools
{

    public ImageTools()
    {
    }

    public static BufferedImage rotate90(BufferedImage in, int direction)
    {
        int width = in.getWidth();
        int height = in.getHeight();
        BufferedImage out = new BufferedImage(height, width, in.getType());
        int line[] = new int[width];
        for(int y = 0; y < height; y++)
        {
            line = in.getRGB(0, y, width, 1, line, 0, width);
            reverseIntArray(line);
            out.setRGB(y, 0, 1, width, line, 0, 1);
        }

        return out;
    }

    private static void reverseIntArray(int a[])
    {
        int j = a.length - 1;
        int halfway = a.length / 2;
        for(int i = 0; i < halfway;)
        {
            int temp = a[i];
            a[i] = a[j];
            a[j] = temp;
            i++;
            j--;
        }

    }

    public static final int LEFT = 0;
    public static final int RIGHT = 1;

}