/**

$Id: Rankable.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;


public interface Rankable
{

    public abstract Object getPlayerAt(int i);

    public abstract String getRankText(int i);
}