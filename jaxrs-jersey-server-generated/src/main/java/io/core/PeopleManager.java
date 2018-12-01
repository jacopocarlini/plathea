/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package io.core;

import static io.core.MainSystem.rooms;
import io.swagger.model.IdentifiedPerson;
import io.swagger.model.Person;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 *
 * @author jack1
 */
public class PeopleManager {
    public int ID = 0;
    private HashMap<Integer,InfoRoom> stanze;
    private HashMap<Integer, Porte> mappa;

    public PeopleManager() {
        this.stanze = new HashMap<Integer, InfoRoom>();
        //this.spariti = new HashMap<Integer, InfoRoom>();
        this.mappa = new HashMap<Integer, Porte>();
    }
    
    
    public void initialize(){
        // leggere file di configurazione della mappa con i punti di contatto delle stanze
    }
    
    public synchronized List<IdentifiedPerson> getPeople(){
        ArrayList<IdentifiedPerson> ret = new ArrayList<IdentifiedPerson>();
        for(Integer key : stanze.keySet()){
            for(MyPerson person : stanze.get(key).presenti){
                ret.add(person.person);
            }     
        }
        return ret;
    }
    
    
    public synchronized IdentifiedPerson getPersonWithIdentity(int globalID){
        for(Integer key : stanze.keySet()){
            for(MyPerson person : stanze.get(key).presenti){
                if(person.globalID == globalID && person.person.isIdentified()){
                    return person.person;
                }
            }
        }
        return null;
    }
    
    public synchronized Person getPerson(int globalID){
        for(Integer key : stanze.keySet()){
            for(MyPerson person : stanze.get(key).presenti){
                if(person.globalID == globalID){
                    return person.person;
                }
            }
        }
        return null;
    }
    
    public synchronized List<IdentifiedPerson> getPeopleInARoom(int roomID){
        ArrayList<MyPerson> present = stanze.get(roomID).presenti;
        int n = present.size();
        List<IdentifiedPerson> ret = new ArrayList<IdentifiedPerson>(n);
        for(int i =0; i<n; i++){
            IdentifiedPerson p = present.get(i).person;
            p.setId(present.get(i).globalID);
            ret.add(p);
        }
        return ret;
    }
    
    public synchronized void updatePeopleInARoom(IdentifiedPerson [] persons){
        int i = persons[0].getPosition().getRoomID();
        stanze.get(i).spariti = difference(persons);
        ArrayList<IdentifiedPerson> people = (ArrayList<IdentifiedPerson>) Arrays.asList(persons);
        ArrayList<MyPerson> ret = new ArrayList();
        //persone.add(new MyPerson(person, ID++));
        for (IdentifiedPerson p : people) {
            ret.add(new MyPerson(p, ID++));
        }
        stanze.get(i).presenti = ret;        
        checkOtherRoom(i);
    }
       
    
    //ritorna le persone che spariscono da una stanza (differenza tra i 2 insiemi)
    private synchronized ArrayList<MyPerson> difference(Person[] newPresent){
        ArrayList<MyPerson> ret = new ArrayList<>();
        ArrayList<Person> pres = (ArrayList<Person>) Arrays.asList(newPresent);

        int i = newPresent[0].getPosition().getRoomID();
        for(MyPerson p : stanze.get(i).presenti){
            if(!pres.contains(p.person)){
                ret.add(p);
            }
        }
        return ret;
    }
    
    // fare un check anche sul timestamp
    private synchronized void checkOtherRoom(int roomid){
        for(MyPerson elem : stanze.get(roomid).presenti){
            for(int i=0; i < mappa.get(roomid).coordsEntrata.size(); i++){
                if(elem.person.getPosition().getX() == mappa.get(roomid).coordsEntrata.get(i).x
                    && elem.person.getPosition().getY()==mappa.get(roomid).coordsEntrata.get(i).y){ //mettere un certo range di incertezza
                    int otherroom = mappa.get(roomid).porte.get(i);
                    for(int j=0;j< stanze.get(otherroom).spariti.size(); j++){
                        Person p = stanze.get(otherroom).spariti.get(j).person;
                        if(p.getPosition().getX() == mappa.get(roomid).coordsUscita.get(i).x
                            && p.getPosition().getY() == mappa.get(roomid).coordsUscita.get(i).y){//mettere un certo range di incertezza
                        elem.globalID = stanze.get(otherroom).spariti.get(j).globalID;
                        } 
                    }
                }                
            }
        }
    }

public class MyPerson{
    public int globalID;
    IdentifiedPerson person;
    int lastroom;
    
    MyPerson(IdentifiedPerson p, int id){
        this.person = p;
        this.globalID = id;
        this.lastroom = p.getPosition().getRoomID();
    }
}

public class Porte{
    public int roomID;
    public ArrayList<Coordinata> coordsUscita = new ArrayList<Coordinata>();
    public ArrayList<Integer> porte = new ArrayList<Integer>(); 
    public ArrayList<Coordinata> coordsEntrata = new ArrayList<Coordinata>();

}


public class Coordinata{
    public int x,y;
}


public class InfoRoom{
    public ArrayList<MyPerson> presenti;
    public ArrayList<MyPerson> spariti;
    
    public InfoRoom(){
        presenti = new ArrayList<>();
        spariti = new ArrayList<>();

    }
}

}