package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.EntitiesApiService;
import io.swagger.api.factories.EntitiesApiServiceFactory;

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

@Path("/entities")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class EntitiesApi  {
   private final EntitiesApiService delegate;

   public EntitiesApi(@Context ServletConfig servletContext) {
      EntitiesApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("EntitiesApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (EntitiesApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = EntitiesApiServiceFactory.getEntitiesApi();
      }

      this.delegate = delegate;
   }

    @GET
    
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all the objects", description = "Return an array of objects", tags={ "entitiy" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(array = @ArraySchema(schema = @Schema(implementation = Entity.class)))),
        
        @ApiResponse(responseCode = "400", description = "Something went wrong") })
    public Response getAllPositions(@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getAllPositions(securityContext);
    }
}
