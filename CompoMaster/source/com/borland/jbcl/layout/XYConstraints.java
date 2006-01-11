// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:14:26
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   XYConstraints.java

package com.borland.jbcl.layout;

import java.io.Serializable;

public class XYConstraints
    implements Cloneable, Serializable
{

    public XYConstraints()
    {
        this(0, 0, 0, 0);
    }

    public XYConstraints(int x, int y, int width, int height)
    {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    public int getX()
    {
        return x;
    }

    public void setX(int x)
    {
        this.x = x;
    }

    public int getY()
    {
        return y;
    }

    public void setY(int y)
    {
        this.y = y;
    }

    public int getWidth()
    {
        return width;
    }

    public void setWidth(int width)
    {
        this.width = width;
    }

    public int getHeight()
    {
        return height;
    }

    public void setHeight(int height)
    {
        this.height = height;
    }

    public int hashCode()
    {
        return x ^ y * 37 ^ width * 43 ^ height * 47;
    }

    public boolean equals(Object that)
    {
        if(that instanceof XYConstraints)
        {
            XYConstraints other = (XYConstraints)that;
            return other.x == x && other.y == y && other.width == width && other.height == height;
        } else
        {
            return false;
        }
    }

    public Object clone()
    {
        return new XYConstraints(x, y, width, height);
    }

    public String toString()
    {
        return String.valueOf(String.valueOf((new StringBuffer("XYConstraints[")).append(x).append(",").append(y).append(",").append(width).append(",").append(height).append("]")));
    }

    int x;
    int y;
    int width;
    int height;
}