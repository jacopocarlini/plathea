package io.swagger.api.impl;

import io.core.MainSystem;
import io.swagger.api.*;
import io.swagger.model.*;

import io.swagger.model.Body;
import io.swagger.model.Body1;
import java.io.File;
import io.swagger.model.Person;
import io.swagger.model.Room;
import io.swagger.model.RoomSettings;
import io.swagger.model.Stream;
import java.util.UUID;

import java.util.Map;
import java.util.List;
import io.swagger.api.NotFoundException;

import java.io.InputStream;

import org.glassfish.jersey.media.multipart.FormDataContentDisposition;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.validation.constraints.*;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T15:23:40.200Z[GMT]")public class RoomApiServiceImpl extends RoomApiService {
    @Override
    public Response addRoom(Body1 body, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.addRoom(body.getRoomName());
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
    @Override
    public Response deleteRoom(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.deleteRoom(roomID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
    @Override
    public Response externalcalibration(Integer roomID, List<File> filename, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnPeopleMessage ret = MainSystem.getPeopleInRoom(roomID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response getPeopleInRoom(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.getRoomInfo(roomID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response getRoomInfo(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.getRoomInfo(roomID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response getRoomSettings(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response getStreams(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnStreamsMessage ret = MainSystem.getStreams((roomID));
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response initializesystem(Integer roomID, UUID username, UUID password, UUID type, UUID resolution, Integer fps, UUID cameraModel, UUID ipAddress1, Integer port1, UUID ipAddress2, Integer port2, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response internalcalibration(Integer roomID, List<File> filename, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response loadconfigurationfile(Integer roomID, Object body, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.loadconfigurationfile(roomID, body);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response plathearecorder(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response plathearecorderstart(Integer roomID, List<File> filename, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response plathearecorderstop(Integer roomID, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response selectsvmclassifier(Integer roomID, Object body, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response startlocalizationengine(Integer roomID, Boolean withoutTracking, Boolean saveTracksToFile, SecurityContext securityContext) throws NotFoundException {
        // do some magic!
        return Response.ok().entity(new ApiResponseMessage(ApiResponseMessage.OK, "magic!")).build();
    }
    @Override
    public Response updateRoom(Body body, Integer roomID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnRoomMessage ret = MainSystem.updateRoom(roomID, body.getRoomName());
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
}
