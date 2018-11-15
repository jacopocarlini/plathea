package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.EntitiyApiService;
import io.swagger.api.factories.EntitiyApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

import io.swagger.model.Entity;

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

@Path("/entitiy")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class EntitiyApi  {
   private final EntitiyApiService delegate;

   public EntitiyApi(@Context ServletConfig servletContext) {
      EntitiyApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("EntitiyApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (EntitiyApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = EntitiyApiServiceFactory.getEntitiyApi();
      }

      this.delegate = delegate;
   }

    @GET
    @Path("/{entityID}")
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all information of an object", description = "Return an object given an objectID", tags={ "entity" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = Entity.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Object not found") })
    public Response getPositionFromEntityID(@Parameter(description = "ID of an object",required=true) @PathParam("entityID") Integer entityID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getPositionFromEntityID(entityID,securityContext);
    }
}
