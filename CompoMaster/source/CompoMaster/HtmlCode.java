/**

$Id: HtmlCode.java,v 1.2 2006/11/27 15:15:45 vvd0 Exp $

**/

package CompoMaster;

import Data.Data;
import Data.MatchList;
import java.awt.*;
import java.awt.datatransfer.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.util.EventObject;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.text.JTextComponent;

// Referenced classes of package CompoMaster:
//            CompoMaster

public class HtmlCode extends JFrame
{
    class CL
        implements ChangeListener, ActionListener, ClipboardOwner
    {

        public void stateChanged(ChangeEvent e)
        {
            updateGui();
        }

        public void actionPerformed(ActionEvent e)
        {
            Object src = e.getSource();
            if(src == btnOk)
                okPressed();
        }

        public void lostOwnership(Clipboard clipboard1, Transferable transferable)
        {
        }

        CL()
        {
        }
    }


    public HtmlCode(Data data)
    {
        gridBagLayout1 = new GridBagLayout();
        txaHtml = new JTextArea();
        jLabel1 = new JLabel();
        jLabel2 = new JLabel();
        sldHor = new JSlider();
        sldVer = new JSlider();
        lblHor = new JLabel();
        lblVer = new JLabel();
        btnOk = new JButton();
        this.data = data;
        try
        {
            jbInit();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    private void jbInit()
        throws Exception
    {
        border1 = BorderFactory.createLineBorder(Color.black, 2);
        getContentPane().setLayout(gridBagLayout1);
        txaHtml.setBorder(border1);
        txaHtml.setToolTipText("");
        jLabel1.setText("Horizontal size:");
        jLabel2.setText("Vertical size:");
        btnOk.setText("Copy to clipboard and close");
        getContentPane().add(txaHtml, new GridBagConstraints(0, 3, 4, 1, 1.0D, 1.0D, 15, 1, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(jLabel1, new GridBagConstraints(0, 0, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(jLabel2, new GridBagConstraints(0, 1, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(sldHor, new GridBagConstraints(1, 0, 1, 1, 1.0D, 0.0D, 10, 2, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(sldVer, new GridBagConstraints(1, 1, 1, 1, 0.0D, 0.0D, 10, 2, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(lblHor, new GridBagConstraints(2, 0, 2, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        getContentPane().add(lblVer, new GridBagConstraints(2, 1, 1, 1, 0.0D, 0.0D, 17, 0, new Insets(0, 0, 0, 0), 0, 0));
        sldVer.setMinimum(0);
        sldVer.setMaximum(100);
        sldVer.setMajorTickSpacing(10);
        sldVer.setValue(100);
        sldVer.setPaintTicks(true);
        sldHor.setMinimum(0);
        sldHor.setMaximum(100);
        sldHor.setMajorTickSpacing(10);
        sldHor.setValue(100);
        sldHor.setPaintTicks(true);
        listener = new CL();
        sldHor.addChangeListener(listener);
        sldVer.addChangeListener(listener);
        btnOk.addActionListener(listener);
        setTitle("Generate HTML code");
        setIconImage(CompoMaster.getIcon());
        updateGui();
        pack();
    }

    private void updateHtml()
    {
        Dimension d = data.matchList.getDrawingSize();
        int h = (d.height * sldVer.getValue()) / 100;
        int w = (d.width * sldHor.getValue()) / 100;
        File ftemp = new File(CompoMaster.dataFileName);
        String txt = new String((
			new StringBuffer("<APPLET archive=\"CompoViewer.jar\" code=\"CompoViewer/CompoViewer.class\"height=\""))
				.append(h).append("\" width=\"").append(w).append("\">\n")
				.append("  <PARAM name=\"DataFile0\" value=\"").append(ftemp.getName())
				.append("\">\n").append("</APPLET>"));
        txaHtml.setText(txt);
    }

    private void updateGui()
    {
        lblHor.setText(new String((new StringBuffer(sldHor.getValue())).append("%")));
        lblVer.setText(new String((new StringBuffer(sldVer.getValue())).append("%")));
        updateHtml();
        getContentPane().add(btnOk, new GridBagConstraints(1, 4, 1, 1, 0.0D, 0.0D, 10, 0, new Insets(0, 0, 0, 0), 0, 0));
    }

    private void okPressed()
    {
        Clipboard c = Toolkit.getDefaultToolkit().getSystemClipboard();
        c.setContents(new StringSelection(txaHtml.getText()), listener);
        dispose();
    }

    Data data;
    GridBagLayout gridBagLayout1;
    JTextArea txaHtml;
    Border border1;
    JLabel jLabel1;
    JLabel jLabel2;
    JSlider sldHor;
    JSlider sldVer;
    JLabel lblHor;
    JLabel lblVer;
    JButton btnOk;
    CL listener;


}