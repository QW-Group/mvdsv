/**

$Id: BufferedReaderTap.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

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