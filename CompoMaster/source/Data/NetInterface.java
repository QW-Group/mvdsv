/**

$Id: NetInterface.java,v 1.2 2006/11/27 15:15:48 vvd0 Exp $

**/

package Data;

import java.io.*;
import java.net.*;

// Referenced classes of package Data:
//            DataInterface, BufferedReaderTap, Data

public class NetInterface extends DataInterface
{

    public NetInterface()
    {
    }

    public static void main(String args[])
    {
        System.setProperty("line.separator", "\r\n");
        boolean ok = startServer(args);
        if(!ok)
        {
            System.out.println("Unable to continue");
            System.exit(0);
        }
        BufferedReader systemIn = new BufferedReader(new InputStreamReader(System.in));
        int c = 0;
        System.out.println("Type Q + enter to quit.");
        try
        {
            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
            String line = "";
            do
            {
                if(line.equals("DEBUG"))
                    toggleDebug();
                if(line.equals("HELP"))
                    showHelp();
                System.out.print("> ");
            } while(!(line = in.readLine().toUpperCase()).equals("Q"));
        }
        catch(IOException ioexception) { }
        System.out.println("Stopping server.");
        System.exit(0);
    }

    private static void showHelp()
    {
        System.out.println(helpStr);
    }

    private static void toggleDebug()
    {
        debug = !debug;
        if(tap != null)
            tap.setEnabled(debug);
        System.out.println("Debug mode: ".concat(String.valueOf(debug)));
    }

    private static boolean startServer(String args[])
    {
        System.out.println(Data.VERSION);
        System.out.println("Initializing command line net server...");
        if(args.length > 0)
            try
            {
                port = Integer.parseInt(args[0]);
            }
            catch(NumberFormatException e)
            {
                System.out.println(new String((new StringBuffer("Unable to set port '")).append(args[0]).append("'")));
            }
        try
        {
            outSocket = new ServerSocket(port);
            System.out.println("Listening on port ".concat(String.valueOf(port)));
            Thread listen = new Thread() {

                public void run()
                {
                    do
                    {
                        try
                        {
                            NetInterface.inSocket = NetInterface.outSocket.accept();
                            InetAddress address = NetInterface.inSocket.getInetAddress();
                            System.out.println("Received connection from ".concat(address.getHostAddress()));
                            DataInterface.outStream = new PrintStream(NetInterface.inSocket.getOutputStream());
                            DataInterface.inStream = new BufferedReaderTap(new InputStreamReader(NetInterface.inSocket.getInputStream()), System.out);
                            NetInterface.tap = (BufferedReaderTap)DataInterface.inStream;
                            NetInterface.tap.setEnabled(NetInterface.debug);
                            DataInterface.init(null);
                            DataInterface.readLoop();
                        }
                        catch(IOException ioexception) { }
                        catch(NullPointerException e)
                        {
                            e.printStackTrace();
                        }
                        System.out.println("Connection terminated.");
                    } while(true);
                }

            };
            listen.start();
            boolean flag = true;
            return flag;
        }
        catch(IOException e)
        {
            System.out.println("Unable to start server on port ".concat(String.valueOf(port)));
        }
        boolean flag1 = false;
        return flag1;
    }

    protected static void terminate()
    {
        System.out.println("Terminating connection");
        if(inSocket != null)
            try
            {
                inSocket.close();
            }
            catch(IOException ioexception) { }
    }

    public static final int ESC = 27;
    private static ServerSocket outSocket;
    private static int port = 9322;
    private static Socket inSocket;
    private static boolean debug = false;
    private static BufferedReaderTap tap;
    private static String helpStr = "\r\nAvailable commands:\r\n\r\nDEBUG      # Toggles debug mode - all network input is echoed.\r\nHELP       # Displays this text\r\nQ          # Quits program\r\n";







}