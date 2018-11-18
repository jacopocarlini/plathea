/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package io.core;

import io.swagger.model.Body1;
import io.swagger.model.Body2;
import io.swagger.model.Body5;
import io.swagger.model.Body6;
import io.swagger.model.Body7;
import io.swagger.model.Body8;
import io.swagger.model.Body9;
import io.swagger.model.Entity;
import io.swagger.model.Identity;
import io.swagger.model.Person;
import io.swagger.model.Position;
import io.swagger.model.Room;
import io.swagger.model.Stream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.ws.rs.Path;

/**
 *
 * @author jack1
 */
public class MainSystem {
    static Map<Integer, MyRoom> rooms = new HashMap<Integer, MyRoom>();
    static Map<Integer, Stream> streams = new HashMap<Integer, Stream>();
    static Map<Integer, Entity> entities = new HashMap<Integer, Entity>();
    static Map<Integer, Identity> identities = new HashMap<Integer, Identity>();
    //static Map<Integer, Person> people = new HashMap<Integer, Person>();


    static int IDs=0;
    static int identityIDs=0;

    public static ReturnRoomMessage initializesystem(Integer roomID, UUID username, UUID password, UUID type, UUID resolution, Integer fps, UUID cameraModel, UUID ipAddress1) {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

              

    //classi wrapper contenenti le info da inviare al client
    public static class ReturnRoomMessage{
        private String message;
        private Room payload;
        private StatusCode code;

        public ReturnRoomMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Room ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnRoomMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Room getPayload() {
            return payload;
        }

        public ReturnRoomMessage setPayload(Room payload) {
            this.payload = payload;
            return this;
        }
        
        
        
    }
    public static class ReturnRoomsMessage{
        private String message;
        private Map<Integer, Room> payload;
        private StatusCode code;

        public ReturnRoomsMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }            
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnRoomsMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Map<Integer, Room> getPayload() {
            return payload;
        }

        public ReturnRoomsMessage setPayload(Map<Integer, Room> payload) {
            this.payload = payload;
            return this;
        }

       
        
        
        
    }
    public static class ReturnPeopleMessage{
        private String message;
        private List<Person> payload;
        private StatusCode code;

        public ReturnPeopleMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnPeopleMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public List<Person> getPayload() {
            return payload;
        }

        public ReturnPeopleMessage setPayload(List<Person> payload) {
            this.payload = payload;
            return this;
        }
        
        
        
    }
    public static class ReturnStreamsMessage{
        private String message;
        private List<Integer> payload;
        private StatusCode code;

        public ReturnStreamsMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Stream ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }


        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnStreamsMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public List<Integer> getPayload() {
            return payload;
        }

        public ReturnStreamsMessage setPayload(List<Integer> payload) {
            this.payload = payload;
            return this;
        }
                
    }
    public static class ReturnStreamMessage{
        private String message;
        private Object payload; //TODO: gestire il flusso video
        private StatusCode code;

        public ReturnStreamMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Stream ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }


        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnStreamMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Object getPayload() {
            return payload;
        }

        public ReturnStreamMessage setPayload(Object payload) {
            this.payload = payload;
            return this;
        }
                
    }
    public static class ReturnPositionMessage{
        private String message;
        private Entity payload; //TODO: gestire il flusso video
        private StatusCode code;
        
        public ReturnPositionMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Identity ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }


        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnPositionMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Entity getPayload() {
            return payload;
        }

        public ReturnPositionMessage setPayload(Entity payload) {
            this.payload = payload;
            return this;
        }
                
    }
    public static class ReturnEntitiesMessage{
        private String message;
        private Map<Integer, Entity> payload;
        private StatusCode code;

        public ReturnEntitiesMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }            
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnEntitiesMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Map<Integer, Entity> getPayload() {
            return payload;
        }

        public ReturnEntitiesMessage setPayload(Map<Integer, Entity> payload) {
            this.payload = payload;
            return this;
        }

       
        
        
        
    }
    public static class ReturnIdentityMessage{
        private String message;
        private Identity payload;
        private StatusCode code;

        public ReturnIdentityMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Identity ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnIdentityMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Identity getPayload() {
            return payload;
        }

        public ReturnIdentityMessage setPayload(Identity payload) {
            this.payload = payload;
            return this;
        }
        
        
        
    }
    public static class ReturnIdentitiesMessage{
        private String message;
        private Map<Integer, Identity> payload;
        private StatusCode code;

        public ReturnIdentitiesMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnIdentitiesMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Map<Integer, Identity> getPayload() {
            return payload;
        }

        public ReturnIdentitiesMessage setPayload(Map<Integer, Identity> payload) {
            this.payload = payload;
            return this;
        }
        
        
        
    }
    public static class ReturnPersonMessage{
        private String message;
        private Person payload;
        private StatusCode code;

        public ReturnPersonMessage(StatusCode code) {            
            this.code = code;
            if (code==StatusCode.OK){
                this.message = "successful operation";
            }
            if (code==StatusCode.NOT_FOUND){
                this.message = "Person ID not found";
            }
            if (code==StatusCode.INVALID){
                this.message = "Invalid ID supplied";
            }
        }

        public String getMessage() {
            return message;
        }

        public StatusCode getCode() {
            return code;
        }

        public ReturnPersonMessage setMessage(String message) {
            this.message = message;
            return this;
        }

        public void setCode(StatusCode code) {
            this.code = code;
        }

        public Person getPayload() {
            return payload;
        }

        public ReturnPersonMessage setPayload(Person payload) {
            this.payload = payload;
            return this;
        }
        
        
        
    }
    
    

    
    public enum StatusCode{OK, NOT_FOUND, INVALID}

    
    // utils
    static Room MyRoomToSwagger(MyRoom myRoom){
        Room room = new Room();
        room.name(myRoom.name);
        room.id(myRoom.id);
        return room;
    }
    static Map<Integer, Room> MyRoomsToSwagger(Map<Integer, MyRoom> myrooms){
        Map<Integer, Room> r = new HashMap<Integer, Room>();
        for(int i : rooms.keySet()){
            r.put(i, MyRoomToSwagger(rooms.get(i)));
        }
        
        return r;
    }
    
    //RESTapi functions
    public static synchronized ReturnRoomMessage getRoomInfo(int roomID){
        if(roomID<0) return new ReturnRoomMessage(StatusCode.INVALID);
        if (!rooms.containsKey(roomID)) {   
            return new ReturnRoomMessage(StatusCode.NOT_FOUND);
        }
        else{
            return new ReturnRoomMessage(StatusCode.OK).setPayload(MyRoomToSwagger(rooms.get(roomID)));
        }            
    }

    public static synchronized ReturnRoomMessage addRoom(String roomName){
        MyRoom room = new MyRoom(IDs++, roomName);
        rooms.put(room.getId(),room);
        return new ReturnRoomMessage(StatusCode.OK).setMessage(room.toString());
    }

    public static synchronized ReturnRoomMessage deleteRoom(int roomId){
        if(rooms.containsKey(roomId)){
            rooms.remove(roomId);
                new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND);
    }

    public static synchronized ReturnRoomMessage updateRoom(int roomId, String roomName){
        if(rooms.containsKey(roomId)){
            rooms.get(roomId).setName(roomName);
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND);
    }

    public static synchronized ReturnRoomsMessage getRooms() {
        return new ReturnRoomsMessage(StatusCode.OK).setPayload(MyRoomsToSwagger(rooms));
    }
    
    public static synchronized ReturnPeopleMessage getPeopleInRoom(int roomID) {
        /*Map<Integer,Person> ret = new HashMap<Integer, Person>();
        for(Integer key : people.keySet()){
            if(people.get(key).getPosition().getRoomID()==roomID){
                ret.put(key, people.get(key));
            }
        }
        */
        return new ReturnPeopleMessage(StatusCode.OK).setPayload(rooms.get(roomID).people);
        
    }
    
    public static synchronized ReturnStreamsMessage getStreams(Integer roomID) {
       if(roomID<0) return new ReturnStreamsMessage(StatusCode.INVALID);
       return new ReturnStreamsMessage(StatusCode.OK).setPayload(rooms.get(roomID).streams); 
    }
    
    public static synchronized ReturnStreamMessage getStream(Integer streamID){
        if(streamID<0) return new ReturnStreamMessage(StatusCode.INVALID);
        if (!streams.containsKey(streamID)) {   
            return new ReturnStreamMessage(StatusCode.NOT_FOUND);
        }
        else{
            return new ReturnStreamMessage(StatusCode.OK).setPayload(new Object());
        }  
    }
    
    public static synchronized ReturnPositionMessage getPositionFromEntityID(Integer entityID) {
        if(entityID<0) return new ReturnPositionMessage(StatusCode.INVALID);
        if (!entities.containsKey(entityID)) {   
            return new ReturnPositionMessage(StatusCode.NOT_FOUND);
        }
        else{
            return new ReturnPositionMessage(StatusCode.OK).setPayload(entities.get(entityID));
        } 
    }
    
    public static synchronized ReturnEntitiesMessage getAllPositions() {
        return new ReturnEntitiesMessage(StatusCode.OK).setPayload(entities);
    }

    public static synchronized ReturnIdentityMessage addIdentity(Body9 body){
        Identity identity = new Identity();
        identity.firstname(body.getFirstname());
        identity.setSurname(body.getSurname());
        identity.setId(identityIDs++);
        identities.put(identity.getId(),identity);
        return new ReturnIdentityMessage(StatusCode.OK);
    }

    public static synchronized ReturnIdentityMessage deleteIdentity(Integer identityID) {
        if(identities.containsKey(identityID)){
            identities.remove(identityID);
                new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnIdentityMessage(StatusCode.NOT_FOUND);    
    }
    
    public static synchronized ReturnIdentityMessage getIdentityInfo(Integer identityID) {
        if(identityID<0) return new ReturnIdentityMessage(StatusCode.INVALID);
        if (!identities.containsKey(identityID)) {   
            return new ReturnIdentityMessage(StatusCode.NOT_FOUND);
        }
        else{
            return new ReturnIdentityMessage(StatusCode.OK).setPayload(identities.get(identityID));
        }     
    }
    
    public static synchronized ReturnIdentityMessage updateIdentity(Integer identityID, Body8 body) {
        if(identities.containsKey(identityID)){
            identities.get(identityID).firstname(body.getFirstname());
            identities.get(identityID).surname(body.getSurname());
            return new ReturnIdentityMessage(StatusCode.OK);
        }
        return new ReturnIdentityMessage(StatusCode.NOT_FOUND);   
    }
    
    public static synchronized ReturnIdentitiesMessage getIdentities() {
        return new ReturnIdentitiesMessage(StatusCode.OK).setPayload(identities);
    }
    
    public static synchronized ReturnPersonMessage getPerson(Integer personID) {
        if(personID<0) return new ReturnPersonMessage(StatusCode.INVALID);
        for(Integer key : rooms.keySet()){
            for(Person person : rooms.get(key).people){
                if(Objects.equals(person.getId(), personID)){
                    return new ReturnPersonMessage(StatusCode.OK).setPayload(person);
                }
            }
        }
        return new ReturnPersonMessage(StatusCode.NOT_FOUND);              
    }
    
    public static synchronized ReturnPeopleMessage getPersonWithIdentity() {
        List <Person> ret = new ArrayList<Person>();
        for(Integer key : rooms.keySet()){
            for(Person person : rooms.get(key).people){
                //if(person.isIdentified()){
                  //  ret.add(person);
                //}
            }
        }
        return new ReturnPeopleMessage(StatusCode.OK).setPayload(ret);
    }
    
    public static synchronized ReturnPeopleMessage getPeople() {
        List <Person> ret = new ArrayList<Person>();
        for(Integer key : rooms.keySet()){
            for(Person person : rooms.get(key).people){                
                ret.add(person);                
            }
        }
        return new ReturnPeopleMessage(StatusCode.OK).setPayload(ret);
    }
    
    
    //PLATHEA
    public static synchronized ReturnRoomMessage facedatabase(Integer roomID, List<String> filenames, List<File> filename){ 
        if (filename.isEmpty()) return new ReturnRoomMessage(StatusCode.INVALID).setMessage("No files in the body");
        //System.out.println(filenames);
        //System.out.println(filenames.size());
        //System.out.println(filename);
        //System.out.println(filename.size());
        if (filenames.size() != filename.size()) return new ReturnRoomMessage(StatusCode.INVALID).setMessage("Error: names of files");
        if(rooms.containsKey(roomID)){
            new File("room"+roomID+"\\FaceDatabase").mkdirs();
            //String[] name = (String[]) filenames.toArray();
            for(int i=0;i<filenames.size();i++){
                File file = filename.get(i);
                try {
                    String path = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\FaceDatabase\\"+filenames.get(i);                   
                    System.out.println(path);
                    File output = new File(path);            
                    byte[] bytesArray = file.toString().getBytes();
                    
                    output.getParentFile().mkdirs();
                    if(output.createNewFile()){
                        System.out.println("File Created");
                    }else System.out.println("File already exists");                   
                    
                    PrintWriter fw = new PrintWriter(output);
                   
                    for(int j=0;j<bytesArray.length; j++){
                        //System.out.println(bytesArray[j]);
                        fw.println(bytesArray[j]);
                    }
                           
                    fw.close();
                    
                    //FileOutputStream fos = new FileOutputStream(output);     
                    //fos.write(bytesArray);
                    //fos.close();
                    
                    
                } catch (Exception ex) {
                    Logger.getLogger(MainSystem.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
                       
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    
    public static synchronized ReturnRoomMessage loadconfigurationfile(Integer roomID, List<File> filename) {   
        if (filename.isEmpty()) return new ReturnRoomMessage(StatusCode.INVALID);
        if(rooms.containsKey(roomID)){
            //System.out.println(filename.get(0));
            File file = filename.get(0);
            /*
            try {
                new File("room"+roomID).mkdir();
                File output = new File("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\experiment.xml");            
                if(output.createNewFile()){
                    System.out.println("File Created");
                }else System.out.println("File already exists");
                
                String s = file.toString().replace("\\", "/");
                System.out.println(s);
                PrintWriter fw = new PrintWriter(output);
                fw.write(s);
                fw.close();
                /*
                System.out.println("leggo");
                byte[] bytesArray = new byte[(int) file.length()]; 
                FileInputStream fis = new FileInputStream(file);
                fis.read(bytesArray); //read file into bytes[]
                fis.close();
                
                System.out.println("scrivo");
                FileOutputStream fos = new FileOutputStream(output);     
                fos.write(bytesArray);
                fos.close();
                
            } catch (Exception ex) {
                Logger.getLogger(MainSystem.class.getName()).log(Level.SEVERE, null, ex);
            }
            */
            rooms.get(roomID).interfaceJNI.loadConfigurationFile(
                    "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\experiment.xml");
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    public static synchronized ReturnRoomMessage internalcalibration(Integer roomID, List<File> filename, Integer mask) { 
        if (filename.isEmpty()) return new ReturnRoomMessage(StatusCode.INVALID);
        if(rooms.containsKey(roomID)){
            /*
            new File("room"+roomID+"\\InternalCalibration").mkdirs();
            String[] name = {"3DReprojection.xml", "Essential.xml", "Fundamental.xml", 
                "LeftDistortion.xml", "LeftIntrinsics.xml", "mx_LEFT.xml", "mx_RIGHT.xml",
                "my_LEFT.xml", "my_RIGHT.xml", "RightDistortion.xml", "RightIntrinsics.xml",
                "Rotation.xml", "Traslation.xml"};
            for(int i=0;i<filename.size();i++){
                File file = filename.get(i);
                try {
                    String path = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\InternalCalibration\\"+name[i];                   
                    System.out.println(path);
                    File output = new File(path);            
                    output.getParentFile().mkdirs();
                    if(output.createNewFile()){
                        System.out.println("File Created");
                    }else System.out.println("File already exists");

                    String s = file.toString().replace("\\", "/");
                    PrintWriter fw = new PrintWriter(output);
                    fw.write(s);
                    fw.close();
                    System.out.println("File salvato");
                } catch (Exception ex) {
                    Logger.getLogger(MainSystem.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
                    */
            rooms.get(roomID).interfaceJNI.internalCalibration("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\InternalCalibration", mask);
           
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    public static synchronized ReturnRoomMessage externalcalibration(Integer roomID, List<File> filename) {   
        if (filename.isEmpty()) return new ReturnRoomMessage(StatusCode.INVALID);
        if(rooms.containsKey(roomID)){
            /*
            new File("room"+roomID+"\\ExternalCalibration").mkdirs();
            String[] name = {"External_Rotation.xml", "External_Traslation.xml"};
            for(int i=0;i<filename.size();i++){
                File file = filename.get(i);
                try {
                    String path = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\ExternalCalibration\\"+name[i];                   
                    System.out.println(path);
                    File output = new File(path);    
                    if(output.createNewFile()){
                        System.out.println("File Created");
                    }else System.out.println("File already exists");

                    String s = file.toString().replace("\\", "/");
                    System.out.println(s.length());
                    PrintWriter fw = new PrintWriter(output);
                    fw.write(s);
                    fw.close();
                } catch (Exception ex) {
                    Logger.getLogger(MainSystem.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
                    */
            rooms.get(roomID).interfaceJNI.externalCalibration("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\ExternalCalibration");
           
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    public static synchronized ReturnRoomMessage selectsvmclassifier(Integer roomID, List<File> filename) {   
        if (filename.isEmpty()) return new ReturnRoomMessage(StatusCode.INVALID);
        if(rooms.containsKey(roomID)){
            /*
            System.out.println(filename.get(0).getName().length());
            File file = filename.get(0);
            try {
                new File("room"+roomID+"\\Tracking").mkdir();
                File output = new File("room"+roomID+"\\Tracking\\svmclassifier.xml");            
                if(output.createNewFile()){
                    System.out.println("File Created");
                }else System.out.println("File already exists");
                
                String s = file.toString().replace("\\", "/");
                System.out.println(s.length());
                PrintWriter fw = new PrintWriter(output);
                fw.write(s);
                fw.close();
            } catch (Exception ex) {
                Logger.getLogger(MainSystem.class.getName()).log(Level.SEVERE, null, ex);
            }
            */
            rooms.get(roomID).interfaceJNI.selectSVMclassifier("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+roomID+"\\Tracking\\svmclassifier.xml");
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    public static synchronized ReturnRoomMessage initializesystem(Integer roomID, String username, String password, String type, String resolution, Integer fps, String cameraModel, String ipAddress1, Integer port1, String ipAddress2, Integer port2) {
        if(rooms.containsKey(roomID)){
            System.out.println(username);
            System.out.println(fps);
            rooms.get(roomID).interfaceJNI.initializeSystem(username, password, type, resolution, fps, cameraModel, ipAddress1, port1, ipAddress2, port2);
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
    
}
    

    

