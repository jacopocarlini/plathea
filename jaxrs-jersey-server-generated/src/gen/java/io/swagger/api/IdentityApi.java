package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.IdentityApiService;
import io.swagger.api.factories.IdentityApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

import io.swagger.model.Body8;
import io.swagger.model.Body9;
import io.swagger.model.Identity;

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

@Path("/identity")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public class IdentityApi  {
   private final IdentityApiService delegate;

   public IdentityApi(@Context ServletConfig servletContext) {
      IdentityApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("IdentityApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (IdentityApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = IdentityApiServiceFactory.getIdentityApi();
      }

      this.delegate = delegate;
   }

    @POST
    
    @Consumes({ "application/json" })
    
    @Operation(summary = "Add a new identity", description = "", tags={ "identity" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "405", description = "Invalid input") })
    public Response addIdentity(@Parameter(description = "The name and the surname of the Identity to create." ,required=true) Body9 body

,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.addIdentity(body,securityContext);
    }
    @DELETE
    @Path("/{identityID}")
    
    
    @Operation(summary = "Delete an identity", description = "", tags={ "identity" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Identity not found") })
    public Response deleteIdentity(@Parameter(description = "Identity id to delete",required=true) @PathParam("identityID") Integer identityID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.deleteIdentity(identityID,securityContext);
    }
    @GET
    @Path("/{identityID}")
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all the information available about a identity", description = "Return a single identity given a identityID", tags={ "identity" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = Identity.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Identity not found") })
    public Response getIdentityInfo(@Parameter(description = "ID of an identity",required=true) @PathParam("identityID") Integer identityID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getIdentityInfo(identityID,securityContext);
    }
    @PUT
    @Path("/{identityID}")
    @Consumes({ "application/json" })
    
    @Operation(summary = "Update an existing identity", description = "", tags={ "identity" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation"),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Identity not found"),
        
        @ApiResponse(responseCode = "405", description = "Validation exception") })
    public Response updateIdentity(@Parameter(description = "Identity that needs to be modified" ,required=true) Body8 body

,@Parameter(description = "Identity ID to update",required=true) @PathParam("identityID") Integer identityID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.updateIdentity(body,identityID,securityContext);
    }
}
