package io.swagger.api.factories;

import io.swagger.api.EntitiyApiService;
import io.swagger.api.impl.EntitiyApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class EntitiyApiServiceFactory {
    private final static EntitiyApiService service = new EntitiyApiServiceImpl();

    public static EntitiyApiService getEntitiyApi() {
        return service;
    }
}
