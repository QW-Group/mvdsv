/**

$Id: Demo.java,v 1.2 2006/11/27 15:15:47 vvd0 Exp $

**/

package Data;

import java.io.Serializable;

public class Demo
    implements Serializable
{

    public Demo(String a, String b, String c)
    {
        url = a;
        pov = b;
        round = c;
    }

    public String url;
    public String pov;
    public String round;
}