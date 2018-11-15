package io.swagger.api.factories;

import io.swagger.api.StreamApiService;
import io.swagger.api.impl.StreamApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-15T18:26:33.199Z[GMT]")public class StreamApiServiceFactory {
    private final static StreamApiService service = new StreamApiServiceImpl();

    public static StreamApiService getStreamApi() {
        return service;
    }
}
