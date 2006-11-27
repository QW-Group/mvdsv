/**

$Id: CmpFilter.java,v 1.2 2006/11/27 15:15:45 vvd0 Exp $

**/

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