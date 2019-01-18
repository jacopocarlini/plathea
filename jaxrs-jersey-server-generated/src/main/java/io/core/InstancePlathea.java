/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


/**
 *
 * @author jack1
 */

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;


public class InstancePlathea {
   
    
    static {
        //System.loadLibrary("InterfaceJNI_DLL");
        System.load("D:\\github\\plathea\\jaxrs-jersey-server-generated\\src\\main\\java\\io\\core\\InterfaceJNI_DLL.dll");

    }
    
    
     // Interface
    public native void hello();
    public native int loadConfigurationFile(int roomID, String dir);
    public native int internalCalibration(String dir, int mask);
    public native int externalCalibration(String dir);
    public native int selectSVMclassifier(String dir);
    public native int initializeSystem(String username, String password, String type, String resolution, int fps, String cameraModel, String IPAddress1, int port1, String IPAddress2, int port2);
    public native int startLocalizationEngine(boolean withoutTracking, boolean saveTracksToFile, String dir);
    public native int platheaPlayer();
    public native int platheaPlayerStart(String dir);
    public native int platheaPlayerStop();
    public native TrackedPerson[] getTrackedPeople();
    public native byte[] getFrame(int id);
    
    
    public static void main(String[] args) {
        System.out.println("start instance");
        InstancePlathea Plathea = new InstancePlathea();
        Plathea.hello();
        //new InstancePlathea().loadConfigurationFile(1,"D:\\github\\plathea\\jaxrs-jersey-server-generated\\room0\\experiment.xml");
       
        int PORT;
        PORT = Integer.parseInt(args[0]);
        ServerSocket serverSocket = null;
        try {
            serverSocket = new ServerSocket(PORT);
        } catch (IOException ex) {
            Logger.getLogger(InstancePlathea.class.getName()).log(Level.SEVERE, null, ex);
        }
        System.out.println("EchoServer: started ");
        System.out.println("Server Socket: " + serverSocket);
        Socket clientSocket=null;
        BufferedReader in=null;
        PrintWriter out=null;
        try {
            
                // bloccante finchè non avviene una connessione
                clientSocket = serverSocket.accept();
                //System.out.println("Connection accepted: "+ clientSocket);
                // creazione stream di input da clientSocket
                InputStreamReader isr = new InputStreamReader(clientSocket.getInputStream());
                in = new BufferedReader(isr);

                // creazione stream di output su clientSocket
                OutputStreamWriter osw = new OutputStreamWriter(clientSocket.getOutputStream());
                BufferedWriter bw = new BufferedWriter(osw);
                out = new PrintWriter(bw, true);            
            while(true){
                String str = in.readLine();
                System.out.println(str);
                String[] params = str.split(";");
                String ret = null;
                if(str.contains("stop")){
                    out.println("stop");
                    break;
                }
                if (str.contains("loadConfigurationFile")){                    
                    Plathea.loadConfigurationFile(Integer.parseInt(params[1]),params[2]);
                    //loadConfigurationFile(1,"D:\\github\\plathea\\jaxrs-jersey-server-generated\\room0\\experiment.xml");
                }
                if(str.contains("internalCalibration")){
                    Plathea.internalCalibration(params[1],Integer.parseInt(params[2]));
                }
                if(str.contains("externalCalibration")){
                    Plathea.externalCalibration(params[1]);
                }
                if(str.contains("selectSVMclassifier")){
                    Plathea.selectSVMclassifier(params[1]);
                }
                if(str.contains("initializeSystem")){
                    Plathea.initializeSystem(params[1], params[2], params[3], params[4],
                            Integer.parseInt(params[5]), params[6], params[7], Integer.parseInt(params[8]),
                            params[9], Integer.parseInt(params[10]));
                    
                }
                if(str.contains("startLocalizationEngine")){
                    boolean b1=false, b2 = false;
                    if (params[1].equals("1")) b1 = true;
                    if (params[2].equals("1")) b2 = true;
                    String dir = "";
                    if (params.length==4) dir = params[3];
                    Plathea.startLocalizationEngine(b1, b2, dir);
                }
                if(str.contains("platheaPlayer")){
                    Plathea.platheaPlayer();
                }                
                if(str.contains("platheaPlayerStart")){
                    Plathea.platheaPlayerStart(params[1]);
                }                
                if(str.contains("platheaPlayerStop")){
                    Plathea.platheaPlayerStop();
                }                                
                if(str.contains("getTrackedPeople")){
                    System.out.println("get plathea people");
                    TrackedPerson[] tp = Plathea.getTrackedPeople();
                    System.out.println(tp.length);                    
                    ret = ArrayToString(tp);
                    out.println(ret);
                    
                }
                                                
                if(str.contains("getFrame")){
                    byte [] frame = Plathea.getFrame(Integer.parseInt(params[1]));
                    DataOutputStream dOut = new DataOutputStream(clientSocket.getOutputStream());
                    System.out.println("frame lenght: "+frame.length);
                    dOut.writeInt(frame.length); // write length of the message
                    dOut.write(frame);           // write the message
                    ret="done";
                }
                
                if (ret==null || ret == "") out.println("ok");
                ret = null;
            }
        }
        catch (IOException e) {
            System.err.println("Accept failed");
            System.exit(1);
        }
        // chiusura di stream e socket
        System.out.println("EchoServer: closing...");
        out.close();
        try {
            in.close();            
            clientSocket.close();
            serverSocket.close();
        } catch (IOException ex) {
            System.out.println("err");        }
               
        }
    
    
    
    static String ArrayToString(TrackedPerson[] tp){
        String ret = "";
        for(int i=0; i<tp.length;i++){
            if(i+1==tp.length)
                ret+=tp[i].toString();
            else
                ret+=tp[i].toString()+":";
        }
        if(ret=="") ret = ".";
        System.out.println("ret="+ret);
        return ret;
    }


   
    public class TrackedPerson {
        public String name;
        public int nameID;
        public int X,Y;
        public int ID;
        public String type;

        public TrackedPerson(String name, int nameID, int X, int Y, int ID, String type) {
            this.name = name;
            this.nameID = nameID;
            this.X = X;
            this.Y = Y;
            this.ID = ID;
            this.type = type;
        }
        public TrackedPerson(int nameID, int X, int Y, int ID){
            this.nameID = nameID;
            this.X = X;
            this.Y = Y;
            this.ID = ID;
        }
        public TrackedPerson(){
            
        }
        
        public void setIntegers(int nameID, int X, int Y, int ID){
            this.nameID = nameID;
            this.X = X;
            this.Y = Y;
            this.ID = ID;
        }
        
        public void setName(String name){
            this.name = name;
        }
        
        public void setType(String type){
            this.type = type;
        }

        @Override
        public String toString() {
            return name + ";" + nameID + ";" + X + ";" + Y + ";" + ID + ";" + type;
        }
        
        

        
    }
      
}
