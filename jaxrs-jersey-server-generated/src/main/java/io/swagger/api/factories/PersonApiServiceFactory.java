package io.swagger.api.factories;

import io.swagger.api.PersonApiService;
import io.swagger.api.impl.PersonApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class PersonApiServiceFactory {
    private final static PersonApiService service = new PersonApiServiceImpl();

    public static PersonApiService getPersonApi() {
        return service;
    }
}
