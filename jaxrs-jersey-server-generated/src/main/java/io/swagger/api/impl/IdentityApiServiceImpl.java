package io.swagger.api.impl;

import io.core.MainSystem;
import io.swagger.api.*;
import io.swagger.model.*;

import io.swagger.model.Body5;
import io.swagger.model.Body6;
import io.swagger.model.Identity;

import java.util.Map;
import java.util.List;
import io.swagger.api.NotFoundException;

import java.io.InputStream;

import org.glassfish.jersey.media.multipart.FormDataContentDisposition;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.validation.constraints.*;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T15:23:40.200Z[GMT]")public class IdentityApiServiceImpl extends IdentityApiService {
    @Override
    public Response addIdentity(Body8 body, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnIdentityMessage ret = MainSystem.addIdentity(body);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
    @Override
    public Response deleteIdentity(Integer identityID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnIdentityMessage ret = MainSystem.deleteIdentity(identityID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
    @Override
    public Response getIdentityInfo(Integer identityID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnIdentityMessage ret = MainSystem.getIdentityInfo(identityID);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getPayload()).build();
    }
    @Override
    public Response updateIdentity(Body7 body, Integer identityID, SecurityContext securityContext) throws NotFoundException {
        MainSystem.ReturnIdentityMessage ret = MainSystem.updateIdentity(identityID, body);
        if (ret.getCode()==MainSystem.StatusCode.INVALID) 
            return Response.status(Response.Status.BAD_REQUEST).entity(ret.getMessage()).build();
        if (ret.getCode()==MainSystem.StatusCode.NOT_FOUND) 
            return Response.status(Response.Status.NOT_FOUND).entity(ret.getMessage()).build();
        return Response.ok().entity(ret.getMessage()).build();
    }
}
