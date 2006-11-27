/**

$Id: FArrayList.java,v 1.2 2006/11/27 15:15:50 vvd0 Exp $

**/

package nilzorlib.diverse;

import java.io.Serializable;

public class FArrayList
    implements Serializable
{

    public FArrayList()
    {
        content = new Object[10];
        count = 0;
    }

    public FArrayList(int startSize)
    {
        if(startSize > 0)
            content = new Object[startSize];
        else
            content = new Object[10];
        count = 0;
    }

    public FArrayList(Object s[], int length)
    {
        content = s;
        count = length;
    }

    public boolean add(Object o)
    {
        return add(count, o);
    }

    public Object[] getArray()
    {
        return content;
    }

    public boolean add(int idx, Object o)
    {
        if(idx > count)
            return false;
        if(count == content.length)
            expandArrayTo(content.length * 2);
        for(int i = count++; i > idx; i++)
            content[i] = content[i - 1];

        content[idx] = o;
        return true;
    }

    public boolean put(int idx, Object o)
    {
        if(idx < 0)
            return false;
        if(idx >= content.length)
            expandArrayTo(idx + 1);
        content[idx] = o;
        return true;
    }

    public Object get(int idx)
    {
        if(idx < content.length && idx >= 0)
            return content[idx];
        else
            return null;
    }

    public boolean remove(int idx)
    {
        if(idx < 0 || idx >= count)
            return false;
        for(int i = idx; i < count - 1; i++)
            content[i] = content[i + 1];

        content[--count] = null;
        return true;
    }

    public int size()
    {
        return count;
    }

    private void expandArrayTo(int size)
    {
        if(size <= content.length)
        {
            return;
        } else
        {
            Object temp[] = new Object[size];
            System.arraycopy(((Object) (content)), 0, ((Object) (temp)), 0, content.length);
            content = temp;
            return;
        }
    }

    public void shuffle()
    {
        int size = size();
        FArrayList temp = new FArrayList(size);
        for(int i = 0; i < size; i++)
        {
            int rnd = (int)(Math.random() * (double)size);
            if(temp.get(rnd) != null)
                i--;
            else
                temp.put(rnd, get(i));
        }

        for(int i = 0; i < size; i++)
            put(i, temp.get(i));

    }

    private static final long serialVersionUID = 0x26ac83079927b456L;
    private Object content[];
    private int count;

}