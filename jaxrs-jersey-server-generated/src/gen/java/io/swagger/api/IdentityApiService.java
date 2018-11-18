package io.swagger.api;

import io.swagger.api.*;
import io.swagger.model.*;

import org.glassfish.jersey.media.multipart.FormDataContentDisposition;

import io.swagger.model.Body8;
import io.swagger.model.Body9;
import io.swagger.model.Identity;

import java.util.Map;
import java.util.List;
import io.swagger.api.NotFoundException;

import java.io.InputStream;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.validation.constraints.*;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public abstract class IdentityApiService {
    public abstract Response addIdentity(Body9 body,SecurityContext securityContext) throws NotFoundException;
    public abstract Response deleteIdentity(Integer identityID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response getIdentityInfo(Integer identityID,SecurityContext securityContext) throws NotFoundException;
    public abstract Response updateIdentity(Body8 body,Integer identityID,SecurityContext securityContext) throws NotFoundException;
}
