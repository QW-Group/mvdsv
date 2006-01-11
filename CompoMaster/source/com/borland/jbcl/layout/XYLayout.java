// Decompiled by DJ v3.8.8.85 Copyright 2005 Atanas Neshkov  Date: 09.01.2006 22:14:30
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 
// Source File Name:   XYLayout.java

package com.borland.jbcl.layout;

import java.awt.*;
import java.io.Serializable;
import java.util.Hashtable;

// Referenced classes of package com.borland.jbcl.layout:
//            XYConstraints

public class XYLayout
    implements LayoutManager2, Serializable
{

    public XYLayout()
    {
        info = new Hashtable();
    }

    public XYLayout(int width, int height)
    {
        info = new Hashtable();
        this.width = width;
        this.height = height;
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

    public String toString()
    {
        return String.valueOf(String.valueOf((new StringBuffer("XYLayout[width=")).append(width).append(",height=").append(height).append("]")));
    }

    public void addLayoutComponent(String s, Component component1)
    {
    }

    public void removeLayoutComponent(Component component)
    {
        info.remove(component);
    }

    public Dimension preferredLayoutSize(Container target)
    {
        return getLayoutSize(target, true);
    }

    public Dimension minimumLayoutSize(Container target)
    {
        return getLayoutSize(target, false);
    }

    public void layoutContainer(Container target)
    {
        Insets insets = target.getInsets();
        int count = target.getComponentCount();
        for(int i = 0; i < count; i++)
        {
            Component component = target.getComponent(i);
            if(component.isVisible())
            {
                Rectangle r = getComponentBounds(component, true);
                component.setBounds(insets.left + r.x, insets.top + r.y, r.width, r.height);
            }
        }

    }

    public void addLayoutComponent(Component component, Object constraints)
    {
        if(constraints instanceof XYConstraints)
            info.put(component, constraints);
    }

    public Dimension maximumLayoutSize(Container target)
    {
        return new Dimension(0x7fffffff, 0x7fffffff);
    }

    public float getLayoutAlignmentX(Container target)
    {
        return 0.5F;
    }

    public float getLayoutAlignmentY(Container target)
    {
        return 0.5F;
    }

    public void invalidateLayout(Container container)
    {
    }

    Rectangle getComponentBounds(Component component, boolean doPreferred)
    {
        XYConstraints constraints = (XYConstraints)info.get(component);
        if(constraints == null)
            constraints = defaultConstraints;
        Rectangle r = new Rectangle(constraints.x, constraints.y, constraints.width, constraints.height);
        if(r.width <= 0 || r.height <= 0)
        {
            Dimension d = doPreferred ? component.getPreferredSize() : component.getMinimumSize();
            if(r.width <= 0)
                r.width = d.width;
            if(r.height <= 0)
                r.height = d.height;
        }
        return r;
    }

    Dimension getLayoutSize(Container target, boolean doPreferred)
    {
        Dimension dim = new Dimension(0, 0);
        if(width <= 0 || height <= 0)
        {
            int count = target.getComponentCount();
            for(int i = 0; i < count; i++)
            {
                Component component = target.getComponent(i);
                if(component.isVisible())
                {
                    Rectangle r = getComponentBounds(component, doPreferred);
                    dim.width = Math.max(dim.width, r.x + r.width);
                    dim.height = Math.max(dim.height, r.y + r.height);
                }
            }

        }
        if(width > 0)
            dim.width = width;
        if(height > 0)
            dim.height = height;
        Insets insets = target.getInsets();
        dim.width += insets.left + insets.right;
        dim.height += insets.top + insets.bottom;
        return dim;
    }

    private static final long serialVersionUID = 200L;
    int width;
    int height;
    Hashtable info;
    static final XYConstraints defaultConstraints = new XYConstraints();

}