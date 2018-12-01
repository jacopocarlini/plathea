package io.swagger.api.impl;

import io.core.MainSystem;
import io.swagger.api.*;
import io.swagger.model.*;

import java.io.File;

import java.util.Map;
import java.util.List;
import io.swagger.api.NotFoundException;

import java.io.InputStream;

import org.glassfish.jersey.media.multipart.FormDataContentDisposition;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.SecurityContext;
import javax.validation.constraints.*;
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T15:23:40.200Z[GMT]")public class StreamApiServiceImpl extends StreamApiService {
    @Override
    public Response getRoomStream(Integer streamID, SecurityContext securityContext) throws NotFoundException {
        return MainSystem.getStream(streamID);
    }
}
