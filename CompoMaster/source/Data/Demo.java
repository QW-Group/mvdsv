// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:16:15
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   Demo.java

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