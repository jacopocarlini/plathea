package io.swagger.api.factories;

import io.swagger.api.IdentitiesApiService;
import io.swagger.api.impl.IdentitiesApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class IdentitiesApiServiceFactory {
    private final static IdentitiesApiService service = new IdentitiesApiServiceImpl();

    public static IdentitiesApiService getIdentitiesApi() {
        return service;
    }
}
