/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package io.core;

import io.swagger.model.Person;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author jack1
 */
public class MyRoom{
        //public List<Person> people;
        public int id;
        public String name;
        public List<Integer> streams;
        public InterfaceJNI interfaceJNI;
        
        public MyRoom(int id, String name){
            //people = new ArrayList<Person>();
            streams = new ArrayList<Integer>();
            this.id=id;
            this.name=name;
            interfaceJNI = new InterfaceJNI();
        }
/*
    public List<Person> getPeople() {
        return people;
    }

    public void setPeople(List<Person> people) {
        this.people = people;
    }
*/
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public List<Integer> getStreams() {
        return streams;
    }

    public void setStreams(List<Integer> streams) {
        this.streams = streams;
    }

    @Override
    public String toString() {
        return "Room{" + "id=" + id + ", name=" + name + '}';
    }

    
        

    }