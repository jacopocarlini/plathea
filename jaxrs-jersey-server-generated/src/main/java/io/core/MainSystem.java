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
import io.swagger.model.Entity;
import io.swagger.model.Identity;
import io.swagger.model.Person;
import io.swagger.model.Position;
import io.swagger.model.Room;
import io.swagger.model.Stream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

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

    public static synchronized ReturnIdentityMessage addIdentity(Body6 body){
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
    
    public static synchronized ReturnIdentityMessage updateIdentity(Integer identityID, Body5 body) {
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
    
    public static synchronized ReturnRoomMessage loadconfigurationfile(Integer roomID, Object body) {   
        if(rooms.containsKey(roomID)){
            System.out.println(body);
            //rooms.get(roomID).interfaceJNI.loadConfigurationFile(null);
            return new ReturnRoomMessage(StatusCode.OK);
        }
        return new ReturnRoomMessage(StatusCode.NOT_FOUND); 
    }
    
}
    

    

