package io.swagger.api.factories;

import io.swagger.api.PeopleApiService;
import io.swagger.api.impl.PeopleApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class PeopleApiServiceFactory {
    private final static PeopleApiService service = new PeopleApiServiceImpl();

    public static PeopleApiService getPeopleApi() {
        return service;
    }
}
