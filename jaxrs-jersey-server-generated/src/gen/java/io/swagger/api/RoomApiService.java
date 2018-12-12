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
import org.glassfish.jersey.media.multipart.FormDataBodyPart;
import org.glassfish.jersey.media.multipart.FormDataParam;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public abstract class RoomApiService {
    public abstract Response addRoom(Body1 body,SecurityContext securityContext) throws NotFoundException;
    public abstract Response deleteRoom(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response externalcalibration(Integer roomID,List<FormDataBodyPart> bodyParts, FormDataContentDisposition fileDispositions,SecurityContext securityContext) throws NotFoundException;
    public abstract Response facedatabase(Integer roomID, List<FormDataBodyPart> bodyParts, FormDataContentDisposition fileDispositions, SecurityContext securityContext) throws NotFoundException;
    public abstract Response getPeopleInRoom(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getRoomInfo(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getRoomSettings(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getStreams(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response initializesystem(Integer roomID,String username,String password,String type,String resolution,Integer fps,String cameraModel,String ipAddress1,Integer port1,String ipAddress2,Integer port2,SecurityContext securityContext) throws NotFoundException;
    public abstract Response internalcalibration(Integer roomID,Integer mask,List<FormDataBodyPart> bodyParts, SecurityContext securityContext) throws NotFoundException;
    public abstract Response loadconfigurationfile(Integer roomID,InputStream uploadedInputStream, FormDataContentDisposition fileDetail,SecurityContext securityContext) throws NotFoundException;
    public abstract Response platheaplayer(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response platheaplayerstart(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response platheaplayerstop(Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response selectsvmclassifier(Integer roomID,InputStream uploadedInputStream, FormDataContentDisposition fileDetail,SecurityContext securityContext) throws NotFoundException;
    public abstract Response startlocalizationengine(Integer roomID,Integer withoutTracking,Integer saveTracksToFile,SecurityContext securityContext) throws NotFoundException;
    public abstract Response updateRoom(Body body,Integer roomID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response continuousTracking(Integer roomID,SecurityContext securityContext) throws NotFoundException;

}
