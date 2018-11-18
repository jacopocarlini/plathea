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

import java.util.ArrayList;
import java.util.List;


public class InterfaceJNI {
    // import C++ code
    static {
        //System.loadLibrary("PlatheaDLL");
        System.load("D:\\github\\plathea\\jaxrs-jersey-server-generated\\src\\main\\java\\io\\core\\InterfaceJNI_DLL.dll");

    }

    // Singleton
    /*
    private static InterfaceJNI instance = null;

    protected InterfaceJNI() {
    }

    public static InterfaceJNI getInstance(){
        if(instance == null) {
         instance = new InterfaceJNI();
      }
      return instance;
    }
    */

    // Interface
    public native int loadConfigurationFile(String dir);
    public native int internalCalibration(String dir, int mask);
    public native int externalCalibration(String dir);
    public native int selectSVMclassifier(String dir);
    public native int initializeSystem(String username, String password, String type, String resolution, int fps, String cameraModel, String IPAddress1, int port1, String IPAddress2, int port2);
    //public native int startLocalizationEngine(boolean withoutTracking, boolean saveTracksToFile, String dir);
    //public native int platheaRecorder_start(String dir);

    //public native String getPeopleInRoom(int roomID);


    void addRoomJNI() {

    }
/*
    class PersonJNI{
        String firstname;
        String surtname;

        public PersonJNI(){

        }
    }

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
