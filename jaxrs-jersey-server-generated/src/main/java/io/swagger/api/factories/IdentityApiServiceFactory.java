package io.swagger.api.factories;

import io.swagger.api.IdentityApiService;
import io.swagger.api.impl.IdentityApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public class IdentityApiServiceFactory {
    private final static IdentityApiService service = new IdentityApiServiceImpl();

    public static IdentityApiService getIdentityApi() {
        return service;
    }
}
