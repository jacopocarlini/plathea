package io.swagger.api.factories;

import io.swagger.api.EntitiesApiService;
import io.swagger.api.impl.EntitiesApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-17T10:12:31.093Z[GMT]")public class EntitiesApiServiceFactory {
    private final static EntitiesApiService service = new EntitiesApiServiceImpl();

    public static EntitiesApiService getEntitiesApi() {
        return service;
    }
}
