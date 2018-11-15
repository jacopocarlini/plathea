package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.RoomApiService;
import io.swagger.api.factories.RoomApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

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
import org.glassfish.jersey.media.multipart.FormDataParam;

import javax.servlet.ServletConfig;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.ws.rs.*;
import javax.validation.constraints.*;

@Path("/room")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class RoomApi  {
   private final RoomApiService delegate;

   public RoomApi(@Context ServletConfig servletContext) {
      RoomApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("RoomApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (RoomApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = RoomApiServiceFactory.getRoomApi();
      }

      this.delegate = delegate;
   }

    @POST
    
    @Consumes({ "application/json" })
    
    @Operation(summary = "Add a new room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response addRoom(@Parameter(description = "The name of the room to create." ,required=true) Body1 body

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.addRoom(body,securityContext);
    }
    @DELETE
    @Path("/{roomID}")
    
    
    @Operation(summary = "Delete a room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response deleteRoom(@Parameter(description = "Room id to delete",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.deleteRoom(roomID,securityContext);
    }
    @POST
    @Path("/{roomID}/external_calibration")
    @Consumes({ "multipart/form-data" })
    
    @Operation(summary = "Load the external calibration files", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response externalcalibration(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Parameter(description = "")@FormDataParam("filename")  List<File> filename
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.externalcalibration(roomID,filename,securityContext);
    }
    @GET
    @Path("/{roomID}/people")
    
    @Produces({ "application/json" })
    @Operation(summary = "Get the people of a room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(array = @ArraySchema(schema = @Schema(implementation = Person.class)))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response getPeopleInRoom(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getPeopleInRoom(roomID,securityContext);
    }
    @GET
    @Path("/{roomID}")
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all the information available about a room", description = "Return a single room given a roomID", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = Room.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response getRoomInfo(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getRoomInfo(roomID,securityContext);
    }
    @GET
    @Path("/{roomID}/settings")
    
    @Produces({ "application/json" })
    @Operation(summary = "Get the settings of a room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = RoomSettings.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response getRoomSettings(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getRoomSettings(roomID,securityContext);
    }
    @GET
    @Path("/{roomID}/streams")
    
    @Produces({ "application/json" })
    @Operation(summary = "Get the IDs of the streams of a room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = Stream.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response getStreams(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getStreams(roomID,securityContext);
    }
    @POST
    @Path("/{roomID}/initialize_system")
    
    
    @Operation(summary = "Initialize the system", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response initializesystem(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,
@Parameter(description = "" ,required=true)@HeaderParam("username") UUID username

,
@Parameter(description = "" ,required=true)@HeaderParam("password") UUID password

,
@Parameter(description = "" ,required=true)@HeaderParam("type") UUID type

,
@Parameter(description = "" ,required=true)@HeaderParam("resolution") UUID resolution

,
@Parameter(description = "" ,required=true)@HeaderParam("fps") Integer fps

,
@Parameter(description = "" ,required=true)@HeaderParam("cameraModel") UUID cameraModel

,
@Parameter(description = "" ,required=true)@HeaderParam("IPAddress1") UUID ipAddress1

,
@Parameter(description = "" ,required=true)@HeaderParam("port1") Integer port1

,
@Parameter(description = "" ,required=true)@HeaderParam("IPAddress2") UUID ipAddress2

,
@Parameter(description = "" ,required=true)@HeaderParam("port2") Integer port2

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.initializesystem(roomID,username,password,type,resolution,fps,cameraModel,ipAddress1,port1,ipAddress2,port2,securityContext);
    }
    @POST
    @Path("/{roomID}/internal_calibration")
    @Consumes({ "multipart/form-data" })
    
    @Operation(summary = "Load the internal calibration files", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response internalcalibration(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Parameter(description = "")@FormDataParam("filename")  List<File> filename
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.internalcalibration(roomID,filename,securityContext);
    }
    @POST
    @Path("/{roomID}/load_configuration_file")
    @Consumes({ "application/xml" })
    
    @Operation(summary = "Load the configuration file", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response loadconfigurationfile(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Parameter(description = "" ) Object body

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.loadconfigurationfile(roomID,body,securityContext);
    }
    @GET
    @Path("/{roomID}/plathea_recorder")
    
    
    @Operation(summary = "Set the mode of plathea in recorder", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response plathearecorder(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.plathearecorder(roomID,securityContext);
    }
    @POST
    @Path("/{roomID}/plathea_recorder_start")
    @Consumes({ "multipart/form-data" })
    
    @Operation(summary = "start the recorder", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response plathearecorderstart(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Parameter(description = "")@FormDataParam("filename")  List<File> filename
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.plathearecorderstart(roomID,filename,securityContext);
    }
    @GET
    @Path("/{roomID}/plathea_recorder_stop")
    
    
    @Operation(summary = "Stop the recorder", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response plathearecorderstop(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.plathearecorderstop(roomID,securityContext);
    }
    @POST
    @Path("/{roomID}/select_svm_classifier")
    @Consumes({ "application/xml" })
    
    @Operation(summary = "Load the svm classifier file", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response selectsvmclassifier(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,@Parameter(description = "" ) Object body

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.selectsvmclassifier(roomID,body,securityContext);
    }
    @POST
    @Path("/{roomID}/start_localization_engine")
    
    
    @Operation(summary = "Start the localization engine", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response startlocalizationengine(@Parameter(description = "ID of a room",required=true) @PathParam("roomID") Integer roomID
,
@Parameter(description = "disable tracking" ,required=true)@HeaderParam("withoutTracking") Boolean withoutTracking

,
@Parameter(description = "save tracks to file" ,required=true)@HeaderParam("saveTracksToFile") Boolean saveTracksToFile

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.startlocalizationengine(roomID,withoutTracking,saveTracksToFile,securityContext);
    }
    @PUT
    @Path("/{roomID}")
    @Consumes({ "application/json" })
    
    @Operation(summary = "Update an existing room", description = "", tags={ "room" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found"),
        
        @ApiResponse(responseCode = "405", description = "Validation exception") })
    public Response updateRoom(@Parameter(description = "Room that needs to be modified" ,required=true) Body body

,@Parameter(description = "Room id to update",required=true) @PathParam("roomID") Integer roomID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.updateRoom(body,roomID,securityContext);
    }
}
