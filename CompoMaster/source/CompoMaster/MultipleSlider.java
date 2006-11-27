/**

$Id: MultipleSlider.java,v 1.2 2006/11/27 15:15:46 vvd0 Exp $

**/

package CompoMaster;

import java.awt.*;
import java.awt.event.*;
import java.util.AbstractCollection;
import java.util.ArrayList;
import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class MultipleSlider extends JComponent
    implements MouseListener, MouseMotionListener
{
    class SliderDescriptor extends JPanel
    {

        SliderDescriptor(Color c, String text)
        {
            JPanel box = new JPanel();
            box.setBorder(BorderFactory.createLineBorder(Color.black));
            box.setBackground(c);
            Dimension dim = new Dimension(15, 15);
            box.setMinimumSize(dim);
            box.setPreferredSize(dim);
            add(box);
            add(new JLabel(text));
        }
    }


    public MultipleSlider()
    {
        this(0, 100, 10, Color.lightGray, 0);
        startAtBackground = true;
        setPreferredSize(new Dimension(200, 30));
    }

    public MultipleSlider(int values[], Color bgcolor, int align)
    {
        startAtBackground = false;
        valueChanged = false;
        sliding = -1;
        this.values = values;
        this.align = align;
        init(bgcolor);
    }

    public void setAligned(int align)
    {
        this.align = align;
    }

    public void addChangeListener(ChangeListener listener)
    {
        changeListeners.add(listener);
    }

    public void removeChangeListener(ChangeListener listener)
    {
        changeListeners.remove(listener);
    }

    private void fireChangeEvents()
    {
        int size = changeListeners.size();
        for(int i = 0; i < size; i++)
            ((ChangeListener)changeListeners.get(i)).stateChanged(new ChangeEvent(this));

    }

    public MultipleSlider(int min, int max, int interval, Color bgcolor, int align)
    {
        startAtBackground = false;
        valueChanged = false;
        sliding = -1;
        this.align = align;
        values = intervalToArray(min, max, interval);
        init(bgcolor);
    }

    private int[] intervalToArray(int min, int max, int interval)
    {
        int values[] = new int[(max - min) / interval + 1];
        int i = 0;
        for(int a = min; a <= max;)
        {
            values[i] = a;
            a += interval;
            i++;
        }

        return values;
    }

    private void init(Color bgcolor)
    {
        changeListeners = new ArrayList();
        areas = new ArrayList();
        selValues = new ArrayList();
        areas.add(bgcolor);
        selValues.add(new Integer(0));
        addMouseListener(this);
        addMouseMotionListener(this);
    }

    public void setValues(int values[])
    {
        this.values = values;
        repaint();
    }

    public void setValues(int min, int max, int interval)
    {
        setValues(intervalToArray(min, max, interval));
    }

    public int[] getValues()
    {
        return values;
    }

    public void addArea(Color color)
    {
        if(startAtBackground)
        {
            areas = new ArrayList();
            startAtBackground = false;
        } else
        {
            selValues.add(new Integer(areas.size() - 1));
        }
        areas.add(color);
        repaint();
    }

    private void reInit()
    {
        ArrayList chl = changeListeners;
        removeMouseListener(this);
        removeMouseMotionListener(this);
        init((Color)areas.get(0));
        changeListeners = chl;
    }

    public void removeAreas()
    {
        reInit();
        startAtBackground = true;
        repaint();
    }

    public int getSelectedValue(int index)
        throws IndexOutOfBoundsException
    {
        int i = ((Integer)selValues.get(index + 1)).intValue();
        return values[i];
    }

    public int getNumSliders()
    {
        return areas.size() - 1;
    }

    public void setSelectedValue(int index, int val)
        throws IndexOutOfBoundsException
    {
        int idx = 0;
        int i = 0;
        do
        {
            if(i >= values.length)
                break;
            if(val == values[i])
            {
                idx = i;
                break;
            }
            i++;
        } while(true);
        selValues.set(index + 1, new Integer(idx));
    }

    public JComponent getDescriptor(int index, String description)
        throws IndexOutOfBoundsException
    {
        return new SliderDescriptor((Color)areas.get(index), description);
    }

    public void paintComponent(Graphics g)
    {
        super.paintComponent(g);
        FontMetrics fm = g.getFontMetrics();
        if(align == 1)
            x_marg = 0;
        else
            x_marg = Math.max(fm.stringWidth(String.valueOf(values[0])), fm.stringWidth(String.valueOf(values[values.length - 1])));
        h_text = 15;
        w = getWidth() - x_marg * 2;
        h = getHeight();
        h_box = h - h_text;
        ivalSize = (double)(w - 1) / (double)(values.length - 1);
        zoneWidth = Math.min(5, (int)ivalSize / 2);
        if(h_box < 0)
            h_box = 0;
        g.setColor(new Color(100, 100, 100));
        g.fillRect(x_marg, 0, w, h_box);
        g.setColor(new Color(0, 0, 0));
        g.drawLine(x_marg, 0, x_marg + w, 0);
        g.drawLine(x_marg, 0, x_marg, h_box);
        g.setColor(Color.white);
        g.drawLine(x_marg, h_box, x_marg + w, h_box);
        g.drawLine(x_marg + w, 0, x_marg + w, h_box);
        g.setColor((Color)areas.get(0));
        g.fillRect(1 + x_marg, 1, w - 2, h_box - 2);
        int numAreas = areas.size();
        for(int i = 0; i < numAreas; i++)
        {
            Color c = (Color)areas.get(i);
            int selIdx = ((Integer)selValues.get(i)).intValue();
            int x_start = (int)((double)selIdx * ivalSize);
            g.setColor(c);
            g.fillRect(1 + x_start + x_marg, 1, w - x_start - 2, h_box - 2);
            g.setColor(Color.black);
            if(i > 0)
            {
                g.setColor(Color.white);
                g.drawLine(x_start + x_marg, 1, x_start + x_marg, h_box - 2);
                g.drawLine((x_start + x_marg) - 1, 1, 1 + x_start + x_marg, 1);
                g.drawLine((x_start + x_marg) - 1, h_box - 2, 1 + x_start + x_marg, h_box - 2);
                g.setColor(Color.lightGray);
                g.drawLine((x_start + x_marg) - 1, 2, (x_start + x_marg) - 1, h_box - 3);
                g.drawLine((x_start + x_marg) - 2, 2, (x_start + x_marg) - 2, 2);
                g.drawLine((x_start + x_marg) - 2, h_box - 3, (x_start + x_marg) - 2, h_box - 3);
                g.setColor(Color.darkGray);
                g.drawLine(x_start + x_marg + 1, 2, x_start + x_marg + 1, h_box - 3);
                g.drawLine(x_start + x_marg + 2, 2, x_start + x_marg + 2, 2);
                g.drawLine(x_start + x_marg + 2, h_box - 3, x_start + x_marg + 2, h_box - 3);
            }
        }

        g.setColor(Color.black);
        for(int i = 0; i < values.length; i++)
        {
            int x_ofs = x_marg + (int)(ivalSize * (double)i);
            String numStr = String.valueOf(values[i]);
            int strWidth = fm.stringWidth(numStr);
            if(align == 1)
                g.drawString(numStr, ((-(int)ivalSize / 2 + x_ofs) - strWidth / 2) + 2, h_box + h_text);
            else
                g.drawString(numStr, (x_ofs - strWidth / 2) + 2, h_box + h_text);
            g.drawLine(x_ofs, h_box + 1, x_ofs, h_box + 3);
        }

    }

    private int zoneHit(Point p, int zoneWidth, boolean all)
    {
        int hit = -1;
        if(p.y < h_box)
        {
            int size;
            int start;
            if(all)
            {
                size = values.length;
                start = 0;
            } else
            {
                size = selValues.size();
                start = 1;
            }
            for(int i = start; i < size; i++)
            {
                int idx;
                if(all)
                    idx = i;
                else
                    idx = ((Integer)selValues.get(i)).intValue();
                int x_start = x_marg + (int)((double)idx * ivalSize);
                if(p.x >= x_start - zoneWidth && p.x <= x_start + zoneWidth)
                    hit = i;
            }

        }
        return hit;
    }

    public void mouseMoved(MouseEvent e)
    {
        if(sliding == -1)
        {
            int hit = zoneHit(e.getPoint(), zoneWidth, false);
            if(hit != -1)
                setCursor(new Cursor(11));
            else
            if(getCursor().getType() == 11)
                setCursor(new Cursor(0));
        }
    }

    public void mouseDragged(MouseEvent e)
    {
        if(sliding != -1)
        {
            int hit = zoneHit(e.getPoint(), (int)(ivalSize / (double)2), true);
            if(hit != -1 && hit != ((Integer)selValues.get(sliding)).intValue())
            {
                boolean ok = true;
                if(sliding > 1)
                    ok &= hit > ((Integer)selValues.get(sliding - 1)).intValue();
                if(sliding < selValues.size() - 1)
                    ok &= hit < ((Integer)selValues.get(sliding + 1)).intValue();
                if(ok)
                {
                    selValues.set(sliding, new Integer(hit));
                    valueChanged = true;
                    repaint();
                }
            }
        }
    }

    public void mousePressed(MouseEvent e)
    {
        sliding = zoneHit(e.getPoint(), zoneWidth, false);
    }

    public void mouseReleased(MouseEvent e)
    {
        sliding = -1;
        if(valueChanged)
        {
            valueChanged = false;
            fireChangeEvents();
        }
    }

    public void mouseClicked(MouseEvent mouseevent)
    {
    }

    public void mouseEntered(MouseEvent mouseevent)
    {
    }

    public void mouseExited(MouseEvent mouseevent)
    {
    }

    public static final int BETWEEN = 1;
    public static final int ALIGNED = 0;
    int values[];
    ArrayList areas;
    ArrayList selValues;
    ArrayList changeListeners;
    int align;
    boolean startAtBackground;
    boolean valueChanged;
    int x_marg;
    int h_text;
    int w;
    int h;
    int h_box;
    int zoneWidth;
    double ivalSize;
    int sliding;

}