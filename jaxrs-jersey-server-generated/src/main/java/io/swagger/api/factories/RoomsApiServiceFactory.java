package io.swagger.api.factories;

import io.swagger.api.RoomsApiService;
import io.swagger.api.impl.RoomsApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class RoomsApiServiceFactory {
    private final static RoomsApiService service = new RoomsApiServiceImpl();

    public static RoomsApiService getRoomsApi() {
        return service;
    }
}
