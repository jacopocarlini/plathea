package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.StreamApiService;
import io.swagger.api.factories.StreamApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

import java.io.File;

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

@Path("/stream")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public class StreamApi  {
   private final StreamApiService delegate;

   public StreamApi(@Context ServletConfig servletContext) {
      StreamApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("StreamApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (StreamApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = StreamApiServiceFactory.getStreamApi();
      }

      this.delegate = delegate;
   }

    @GET
    @Path("/{streamID}")
    
    @Produces({ "application/json" })
    @Operation(summary = "Get a stream of a room", description = "", tags={ "stream" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = File.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Room not found") })
    public Response getRoomStream(@Parameter(description = "ID of a stream",required=true) @PathParam("streamID") Integer streamID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getRoomStream(streamID,securityContext);
    }
}
