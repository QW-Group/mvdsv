// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:15:36
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   NetInterface.java

package Data;

import java.io.*;

class BufferedReaderTap extends BufferedReader
{

    BufferedReaderTap(InputStreamReader source, PrintStream tap)
    {
        super(source);
        tapActive = true;
        this.tap = tap;
    }

    public void setEnabled(boolean enable)
    {
        tapActive = enable;
    }

    public String readLine()
        throws IOException
    {
        String line = super.readLine();
        if(tapActive)
            tap.println(line);
        return line;
    }

    private PrintStream tap;
    private boolean tapActive;
}