package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.PersonApiService;
import io.swagger.api.factories.PersonApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

import io.swagger.model.IdentifiedPerson;
import io.swagger.model.Person;

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

@Path("/person")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class PersonApi  {
   private final PersonApiService delegate;

   public PersonApi(@Context ServletConfig servletContext) {
      PersonApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("PersonApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (PersonApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = PersonApiServiceFactory.getPersonApi();
      }

      this.delegate = delegate;
   }

    @GET
    @Path("/{personID}")
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all information of a person", description = "Return a person given a personID", tags={ "person" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = Person.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Person not found") })
    public Response getPerson(@Parameter(description = "ID of a person",required=true) @PathParam("personID") Integer personID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getPerson(personID,securityContext);
    }
    @GET
    @Path("/{personID}/withIdentity")
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all information of a person", description = "Return a person given a personID", tags={ "person" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(schema = @Schema(implementation = IdentifiedPerson.class))),
        
        @ApiResponse(responseCode = "400", description = "Invalid ID supplied"),
        
        @ApiResponse(responseCode = "404", description = "Person not found") })
    public Response getPersonWithIdentity(@Parameter(description = "ID of a person",required=true) @PathParam("personID") Integer personID
,@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getPersonWithIdentity(personID,securityContext);
    }
}
