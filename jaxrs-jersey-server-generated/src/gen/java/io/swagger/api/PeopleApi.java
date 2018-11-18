package io.swagger.api;

import io.swagger.model.*;
import io.swagger.api.PeopleApiService;
import io.swagger.api.factories.PeopleApiServiceFactory;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.media.ArraySchema;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;

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

@Path("/people")


@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public class PeopleApi  {
   private final PeopleApiService delegate;

   public PeopleApi(@Context ServletConfig servletContext) {
      PeopleApiService delegate = null;

      if (servletContext != null) {
         String implClass = servletContext.getInitParameter("PeopleApi.implementation");
         if (implClass != null && !"".equals(implClass.trim())) {
            try {
               delegate = (PeopleApiService) Class.forName(implClass).newInstance();
            } catch (Exception e) {
               throw new RuntimeException(e);
            }
         } 
      }

      if (delegate == null) {
         delegate = PeopleApiServiceFactory.getPeopleApi();
      }

      this.delegate = delegate;
   }

    @GET
    
    
    @Produces({ "application/json" })
    @Operation(summary = "Returns all existing people", description = "Return an array of people", tags={ "person" })
    @ApiResponses(value = { 
        @ApiResponse(responseCode = "200", description = "successful operation", content = @Content(array = @ArraySchema(schema = @Schema(implementation = Person.class)))),
        
        @ApiResponse(responseCode = "400", description = "Something went wrong") })
    public Response getAllPeople(@Context SecurityContext securityContext)
    throws NotFoundException {
        return delegate.getAllPeople(securityContext);
    }
}
