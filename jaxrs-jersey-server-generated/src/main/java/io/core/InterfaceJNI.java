/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author jack1
 */
package io.core;

import com.google.common.util.concurrent.SimpleTimeLimiter;
import com.google.common.util.concurrent.TimeLimiter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;


public class InterfaceJNI {
    int roomID;
    public Process myProcess;
   
    InetAddress addr ;            
    Socket socket=null;
    BufferedReader in=null, stdIn=null;
    PrintWriter out=null;
     DataInputStream dIn =null;

    
    
    public InterfaceJNI(int roomID){
        this.roomID = roomID;
        
        try {
            String port = String.valueOf(4000+this.roomID);
            //sendCommand("stop");
            Runtime rt = Runtime.getRuntime();    
            //myProcess = rt.exec("java -cp D:\\github\\plathea\\jaxrs-jersey-server-generated\\src\\main\\java\\io\\core InstancePlathea "+port);
        
        
            addr = InetAddress.getByName(null);       
            // creazione socket
            socket = new Socket(addr, 4000+this.roomID);

            System.out.println("EchoClient: started");
            System.out.println("Client Socket: "+ socket);
            
            // creazione stream di input da socket
            InputStreamReader isr = null;
            isr = new InputStreamReader( socket.getInputStream());
            in = new BufferedReader(isr);

            // creazione stream di output su socket
            OutputStreamWriter osw = new OutputStreamWriter( socket.getOutputStream());
            BufferedWriter bw = new BufferedWriter(osw);
            out = new PrintWriter(bw, true);  
            // creazione stream di input da socket 
            dIn = new DataInputStream(socket.getInputStream());
                                
        } catch (Exception ex) {
            System.out.println(ex);
        }
        
    }
    
    public void hello(){
    
    }
    public int loadConfigurationFile(int roomID, String dir){
        sendCommand("loadConfigurationFile;"+roomID+";"+dir);
        return 0;
    }
    public int internalCalibration(String dir, int mask){
        sendCommand("internalCalibration;"+dir+";"+mask);
        return 0;
    }
    public int externalCalibration(String dir){
        sendCommand("externalCalibration;"+dir);
        return 0;
    }
    public int selectSVMclassifier(String dir){
        sendCommand("selectSVMclassifier;"+dir);
        return 0;
    }
    public int initializeSystem(String username, String password, String type, String resolution, int fps, String cameraModel, String IPAddress1, int port1, String IPAddress2, int port2){
        sendCommand("initializeSystem;"+username+";"+password+";"+type+";"+resolution+";"+fps+";"+cameraModel+";"+IPAddress1+";"+port1+";"+IPAddress2+";"+port2);
        return 0;
    }
    public int startLocalizationEngine(boolean withoutTracking, boolean saveTracksToFile, String dir){
        int b1=0,b2=0;
        if(withoutTracking) b1=1;
        if(saveTracksToFile) b2=1;

        sendCommand("startLocalizationEngine;"+b1+";"+b2+";"+dir);
        return 0;
    }
    public int platheaPlayer(){
        sendCommand("platheaPlayer");
        return 0;
    }
    public int platheaPlayerStart(String dir){
        sendCommand("platheaPlayerStart;"+dir);
        return 0;
    }
    public int platheaPlayerStop(){
        sendCommand("platheaPlayerStop");
        return 0;
    }
    public TrackedPerson[] getTrackedPeople(){
        String string = sendCommand("getTrackedPeople");      
        if(!string.contains(";")) return new TrackedPerson[0];
        String[] elem = string.split(":");      
        System.out.println(elem.length+" "+ string);
        TrackedPerson[] ret = new TrackedPerson[elem.length];
        for(int i=0; i < elem.length; i++){
            String[] params = elem[i].split(";");
            TrackedPerson p = new TrackedPerson(params[0], Integer.parseInt(params[1]),
                Integer.parseInt(params[2]), Integer.parseInt(params[3]), Integer.parseInt(params[4]), params[5]);
            ret[i]=p;
        }
        return ret;
    }
    
    public byte[] getFrame(int id){
        return sendSpecialCommand("getFrame;"+id);        
    }
    
    String sendCommand(String cmd){
        String ret = "";
        try {
            /*
            addr = InetAddress.getByName(null);       
            // creazione socket
            socket = new Socket(addr, 4000+this.roomID);

            System.out.println("EchoClient: started");
            System.out.println("Client Socket: "+ socket);
            
            // creazione stream di output su socket
            OutputStreamWriter osw = new OutputStreamWriter( socket.getOutputStream());
            BufferedWriter bw = new BufferedWriter(osw);
            out = new PrintWriter(bw, true);
            */
            out.println(cmd);
            ret = in.readLine();
            System.out.println("Echo: " + ret);
            //out.close();
            //in.close();
            //socket.close(); 
        }catch (Exception e) {
            System.err.println("Couldn't get I/O for the connection to: " + addr);                
        }
           
        return ret;
    }
    
    byte [] sendSpecialCommand(String cmd){        
        try{                      
            
            out.println(cmd);
            int length = dIn.readInt();                    // read length of incoming message
            if(length>0) {
                byte[] message = new byte[length];
                dIn.readFully(message, 0, message.length); // read the message
                System.out.println(message.length);
                return message;
            }
           
            //out.close();
            //dIn.close();
            //socket.close();            
            } catch (Exception e) {
                System.err.println("Couldn't get I/O for the connection to: " + addr);
            }             
        
        return new byte[0];
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

        
    }
    
/*
    class PersonJNI{
        String name;
        int nameID;
        int X,Y;
        int ID;
        String type;
        

        public PersonJNI(){

        }
    }
/*
    public class RoomJNI{
        public List<PersonJNI> people;
        public int id;
        public String name;
        public List<Integer> streams;

        public RoomJNI(int id, String name){
            people = new ArrayList<PersonJNI>();
            streams = new ArrayList<Integer>();
            this.id=id;
            this.name=name;
        }


    }
*/
}

/*
***********COME COMPILARE JNI***********

javac -h . InterfaceJNI.java
x86_64-w64-mingw32-gcc -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" -shared -o core.dll io_library_InterfaceJNI.cpp

*/
