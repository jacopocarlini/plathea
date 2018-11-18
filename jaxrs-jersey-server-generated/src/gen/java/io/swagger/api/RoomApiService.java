package io.swagger.api;

import io.swagger.api.*;
import io.swagger.model.*;

import org.glassfish.jersey.media.multipart.FormDataContentDisposition;

import io.swagger.model.Body;
import io.swagger.model.Body1;
import java.io.File;
import io.swagger.model.Person;
import io.swagger.model.Room;
import io.swagger.model.RoomSettings;
import io.swagger.model.Stream;

import java.util.Map;
import java.util.List;
import io.swagger.api.NotFoundException;

import java.io.InputStream;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.validation.constraints.*;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public abstract class RoomApiService {
    public abstract Response addRoom(Body1 body,SecurityContext securityContext) throws NotFoundException;
    public abstract Response deleteRoom(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response externalcalibration(Integer roomID,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response facedatabase(Integer roomID,List<String> filenames,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getPeopleInRoom(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getRoomInfo(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getRoomSettings(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getStreams(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response initializesystem(Integer roomID,String username,String password,String type,String resolution,Integer fps,String cameraModel,String ipAddress1,Integer port1,String ipAddress2,Integer port2,SecurityContext securityContext) throws NotFoundException;
    public abstract Response internalcalibration(Integer roomID,Integer mask,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response loadconfigurationfile(Integer roomID,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response plathearecorder(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response plathearecorderstart(Integer roomID,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response plathearecorderstop(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response selectsvmclassifier(Integer roomID,List<File> filename,SecurityContext securityContext) throws NotFoundException;
    public abstract Response startlocalizationengine(Integer roomID,Boolean withoutTracking,Boolean saveTracksToFile,SecurityContext securityContext) throws NotFoundException;
    public abstract Response updateRoom(Body body,Integer roomID,SecurityContext securityContext) throws NotFoundException;
}
