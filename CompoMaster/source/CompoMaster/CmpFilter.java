// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:03:54
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   CompoMaster.java

package CompoMaster;

import java.io.File;
import java.io.FilenameFilter;

class CmpFilter
    implements FilenameFilter
{

    CmpFilter()
    {
    }

    public boolean accept(File directory, String filename)
    {
        return filename.endsWith(".cmp");
    }
}